#include "NovelMind/scripting/vm.hpp"
#include "NovelMind/core/logger.hpp"
#include <cstring>

namespace NovelMind::scripting {

VirtualMachine::VirtualMachine()
    : m_ip(0), m_running(false), m_paused(false), m_waiting(false),
      m_halted(false), m_choiceResult(-1) {}

VirtualMachine::~VirtualMachine() = default;

Result<void> VirtualMachine::load(const std::vector<Instruction> &program,
                                  const std::vector<std::string> &stringTable) {
  if (program.empty()) {
    return Result<void>::error("Empty program");
  }

  m_program = program;
  m_stringTable = stringTable;
  reset();

  return Result<void>::ok();
}

void VirtualMachine::reset() {
  m_ip = 0;
  m_stack.clear();
  m_running = false;
  m_paused = false;
  m_waiting = false;
  m_halted = false;
  m_choiceResult = -1;
}

bool VirtualMachine::step() {
  if (m_halted || m_paused || m_waiting) {
    return false;
  }

  if (m_ip >= m_program.size()) {
    m_halted = true;
    return false;
  }

  executeInstruction(m_program[m_ip]);
  ++m_ip;

  return !m_halted;
}

void VirtualMachine::run() {
  m_running = true;
  m_paused = false;

  while (m_running && !m_halted && !m_paused && !m_waiting) {
    step();
  }
}

void VirtualMachine::pause() { m_paused = true; }

void VirtualMachine::resume() {
  m_paused = false;
  if (m_running && !m_waiting) {
    run();
  }
}

bool VirtualMachine::isRunning() const { return m_running; }

bool VirtualMachine::isPaused() const { return m_paused; }

bool VirtualMachine::isWaiting() const { return m_waiting; }

bool VirtualMachine::isHalted() const { return m_halted; }

void VirtualMachine::setVariable(const std::string &name, Value value) {
  m_variables[name] = std::move(value);
}

Value VirtualMachine::getVariable(const std::string &name) const {
  auto it = m_variables.find(name);
  if (it != m_variables.end()) {
    return it->second;
  }
  return std::monostate{};
}

bool VirtualMachine::hasVariable(const std::string &name) const {
  return m_variables.find(name) != m_variables.end();
}

void VirtualMachine::setFlag(const std::string &name, bool value) {
  m_flags[name] = value;
}

bool VirtualMachine::getFlag(const std::string &name) const {
  auto it = m_flags.find(name);
  if (it != m_flags.end()) {
    return it->second;
  }
  return false;
}

void VirtualMachine::registerCallback(OpCode op, NativeCallback callback) {
  m_callbacks[op] = std::move(callback);
}

void VirtualMachine::signalContinue() {
  m_waiting = false;
  if (m_running && !m_paused) {
    run();
  }
}

void VirtualMachine::signalChoice(i32 choice) {
  m_choiceResult = choice;
  m_waiting = false;
  if (m_running && !m_paused) {
    run();
  }
}

void VirtualMachine::executeInstruction(const Instruction &instr) {
  switch (instr.opcode) {
  case OpCode::NOP:
    break;

  case OpCode::HALT:
    m_halted = true;
    break;

  case OpCode::JUMP:
    m_ip = instr.operand - 1; // -1 because we increment after
    break;

  case OpCode::JUMP_IF:
    if (asBool(pop())) {
      m_ip = instr.operand - 1;
    }
    break;

  case OpCode::JUMP_IF_NOT:
    if (!asBool(pop())) {
      m_ip = instr.operand - 1;
    }
    break;

  case OpCode::PUSH_INT:
    push(static_cast<i32>(instr.operand));
    break;

  case OpCode::PUSH_FLOAT: {
    f32 val;
    std::memcpy(&val, &instr.operand, sizeof(f32));
    push(val);
    break;
  }

  case OpCode::PUSH_STRING:
    push(getString(instr.operand));
    break;

  case OpCode::PUSH_BOOL:
    push(instr.operand != 0);
    break;

  case OpCode::PUSH_NULL:
    push(std::monostate{});
    break;

  case OpCode::POP:
    pop();
    break;

  case OpCode::DUP:
    if (!m_stack.empty()) {
      push(m_stack.back());
    }
    break;

  case OpCode::LOAD_VAR: {
    const std::string &name = getString(instr.operand);
    push(getVariable(name));
    break;
  }

  case OpCode::STORE_VAR: {
    const std::string &name = getString(instr.operand);
    setVariable(name, pop());
    break;
  }

  case OpCode::ADD: {
    Value b = pop();
    Value a = pop();
    if (getValueType(a) == ValueType::String ||
        getValueType(b) == ValueType::String) {
      push(asString(a) + asString(b));
    } else if (getValueType(a) == ValueType::Float ||
               getValueType(b) == ValueType::Float) {
      push(asFloat(a) + asFloat(b));
    } else {
      push(asInt(a) + asInt(b));
    }
    break;
  }

  case OpCode::SUB: {
    Value b = pop();
    Value a = pop();
    if (getValueType(a) == ValueType::Float ||
        getValueType(b) == ValueType::Float) {
      push(asFloat(a) - asFloat(b));
    } else {
      push(asInt(a) - asInt(b));
    }
    break;
  }

  case OpCode::MUL: {
    Value b = pop();
    Value a = pop();
    if (getValueType(a) == ValueType::Float ||
        getValueType(b) == ValueType::Float) {
      push(asFloat(a) * asFloat(b));
    } else {
      push(asInt(a) * asInt(b));
    }
    break;
  }

  case OpCode::DIV: {
    Value b = pop();
    Value a = pop();
    f32 divisor = asFloat(b);
    if (divisor != 0.0f) {
      push(asFloat(a) / divisor);
    } else {
      NOVELMIND_LOG_ERROR("Division by zero");
      push(0);
    }
    break;
  }

  case OpCode::EQ: {
    Value b = pop();
    Value a = pop();
    push(asString(a) == asString(b));
    break;
  }

  case OpCode::NE: {
    Value b = pop();
    Value a = pop();
    push(asString(a) != asString(b));
    break;
  }

  case OpCode::LT: {
    Value b = pop();
    Value a = pop();
    push(asFloat(a) < asFloat(b));
    break;
  }

  case OpCode::LE: {
    Value b = pop();
    Value a = pop();
    push(asFloat(a) <= asFloat(b));
    break;
  }

  case OpCode::GT: {
    Value b = pop();
    Value a = pop();
    push(asFloat(a) > asFloat(b));
    break;
  }

  case OpCode::GE: {
    Value b = pop();
    Value a = pop();
    push(asFloat(a) >= asFloat(b));
    break;
  }

  case OpCode::AND: {
    Value b = pop();
    Value a = pop();
    push(asBool(a) && asBool(b));
    break;
  }

  case OpCode::OR: {
    Value b = pop();
    Value a = pop();
    push(asBool(a) || asBool(b));
    break;
  }

  case OpCode::NOT: {
    Value a = pop();
    push(!asBool(a));
    break;
  }

  case OpCode::SET_FLAG: {
    bool value = asBool(pop());
    const std::string &name = getString(instr.operand);
    setFlag(name, value);
    break;
  }

  case OpCode::CHECK_FLAG: {
    const std::string &name = getString(instr.operand);
    push(getFlag(name));
    break;
  }

  case OpCode::SAY:
  case OpCode::SHOW_BACKGROUND:
  case OpCode::SHOW_CHARACTER:
  case OpCode::HIDE_CHARACTER:
  case OpCode::CHOICE:
  case OpCode::PLAY_SOUND:
  case OpCode::PLAY_MUSIC:
  case OpCode::STOP_MUSIC:
  case OpCode::WAIT:
  case OpCode::TRANSITION:
  case OpCode::GOTO_SCENE: {
    auto it = m_callbacks.find(instr.opcode);
    if (it != m_callbacks.end()) {
      std::vector<Value> args;
      // Collect args from stack if needed
      it->second(args);
    }

    // These commands typically wait for user input
    if (instr.opcode == OpCode::SAY || instr.opcode == OpCode::CHOICE ||
        instr.opcode == OpCode::WAIT) {
      m_waiting = true;
    }
    break;
  }

  default:
    NOVELMIND_LOG_WARN("Unknown opcode");
    break;
  }
}

void VirtualMachine::push(Value value) { m_stack.push_back(std::move(value)); }

Value VirtualMachine::pop() {
  if (m_stack.empty()) {
    NOVELMIND_LOG_WARN("Stack underflow");
    return std::monostate{};
  }
  Value val = std::move(m_stack.back());
  m_stack.pop_back();
  return val;
}

const std::string &VirtualMachine::getString(u32 index) const {
  static const std::string empty;
  if (index < m_stringTable.size()) {
    return m_stringTable[index];
  }
  NOVELMIND_LOG_WARN("Invalid string index");
  return empty;
}

} // namespace NovelMind::scripting
