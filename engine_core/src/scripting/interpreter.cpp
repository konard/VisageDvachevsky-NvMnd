#include "NovelMind/scripting/interpreter.hpp"
#include "NovelMind/core/logger.hpp"
#include <cstring>

namespace NovelMind::scripting {

constexpr u32 SCRIPT_MAGIC = 0x43534D4E; // "NMSC"

ScriptInterpreter::ScriptInterpreter()
    : m_vm(std::make_unique<VirtualMachine>()) {}

ScriptInterpreter::~ScriptInterpreter() = default;

Result<void>
ScriptInterpreter::loadFromBytecode(const std::vector<u8> &bytecode) {
  if (bytecode.size() < 20) // Minimum header size
  {
    return Result<void>::error("Bytecode too small");
  }

  usize offset = 0;

  // Read magic
  u32 magic;
  std::memcpy(&magic, bytecode.data() + offset, sizeof(u32));
  offset += sizeof(u32);

  if (magic != SCRIPT_MAGIC) {
    return Result<void>::error("Invalid script magic");
  }

  // Read version
  u16 version;
  std::memcpy(&version, bytecode.data() + offset, sizeof(u16));
  offset += sizeof(u16);
  offset += sizeof(u16); // Skip flags

  // Read instruction count
  u32 instrCount;
  std::memcpy(&instrCount, bytecode.data() + offset, sizeof(u32));
  offset += sizeof(u32);

  // Read constant pool size (skip for now)
  offset += sizeof(u32);

  // Read string table size
  u32 stringCount;
  std::memcpy(&stringCount, bytecode.data() + offset, sizeof(u32));
  offset += sizeof(u32);

  // Skip symbol table size
  offset += sizeof(u32);

  // Read instructions
  std::vector<Instruction> program;
  program.reserve(instrCount);

  for (u32 i = 0; i < instrCount; ++i) {
    if (offset + 5 > bytecode.size()) {
      return Result<void>::error("Unexpected end of bytecode");
    }

    Instruction instr;
    instr.opcode = static_cast<OpCode>(bytecode[offset]);
    offset += 1;

    std::memcpy(&instr.operand, bytecode.data() + offset, sizeof(u32));
    offset += sizeof(u32);

    program.push_back(instr);
  }

  // Read string table
  std::vector<std::string> stringTable;
  stringTable.reserve(stringCount);

  for (u32 i = 0; i < stringCount; ++i) {
    std::string str;
    while (offset < bytecode.size() && bytecode[offset] != 0) {
      str += static_cast<char>(bytecode[offset]);
      ++offset;
    }
    ++offset; // Skip null terminator
    stringTable.push_back(str);
  }

  return m_vm->load(program, stringTable);
}

void ScriptInterpreter::reset() { m_vm->reset(); }

bool ScriptInterpreter::step() { return m_vm->step(); }

void ScriptInterpreter::run() { m_vm->run(); }

void ScriptInterpreter::pause() { m_vm->pause(); }

void ScriptInterpreter::resume() { m_vm->resume(); }

bool ScriptInterpreter::isRunning() const { return m_vm->isRunning(); }

bool ScriptInterpreter::isPaused() const { return m_vm->isPaused(); }

bool ScriptInterpreter::isWaiting() const { return m_vm->isWaiting(); }

void ScriptInterpreter::setVariable(const std::string &name, i32 value) {
  m_vm->setVariable(name, value);
}

void ScriptInterpreter::setVariable(const std::string &name, f32 value) {
  m_vm->setVariable(name, value);
}

void ScriptInterpreter::setVariable(const std::string &name,
                                    const std::string &value) {
  m_vm->setVariable(name, value);
}

void ScriptInterpreter::setVariable(const std::string &name, bool value) {
  m_vm->setVariable(name, value);
}

std::optional<i32>
ScriptInterpreter::getIntVariable(const std::string &name) const {
  if (!m_vm->hasVariable(name)) {
    return std::nullopt;
  }
  Value val = m_vm->getVariable(name);
  if (auto *p = std::get_if<i32>(&val)) {
    return *p;
  }
  return std::nullopt;
}

std::optional<f32>
ScriptInterpreter::getFloatVariable(const std::string &name) const {
  if (!m_vm->hasVariable(name)) {
    return std::nullopt;
  }
  Value val = m_vm->getVariable(name);
  if (auto *p = std::get_if<f32>(&val)) {
    return *p;
  }
  return std::nullopt;
}

std::optional<std::string>
ScriptInterpreter::getStringVariable(const std::string &name) const {
  if (!m_vm->hasVariable(name)) {
    return std::nullopt;
  }
  Value val = m_vm->getVariable(name);
  if (auto *p = std::get_if<std::string>(&val)) {
    return *p;
  }
  return std::nullopt;
}

std::optional<bool>
ScriptInterpreter::getBoolVariable(const std::string &name) const {
  if (!m_vm->hasVariable(name)) {
    return std::nullopt;
  }
  Value val = m_vm->getVariable(name);
  if (auto *p = std::get_if<bool>(&val)) {
    return *p;
  }
  return std::nullopt;
}

void ScriptInterpreter::setFlag(const std::string &name, bool value) {
  m_vm->setFlag(name, value);
}

bool ScriptInterpreter::getFlag(const std::string &name) const {
  return m_vm->getFlag(name);
}

void ScriptInterpreter::signalContinue() { m_vm->signalContinue(); }

void ScriptInterpreter::signalChoice(i32 choice) { m_vm->signalChoice(choice); }

void ScriptInterpreter::registerCallback(
    OpCode op, VirtualMachine::NativeCallback callback) {
  m_vm->registerCallback(op, std::move(callback));
}

} // namespace NovelMind::scripting
