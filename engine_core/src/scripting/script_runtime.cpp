#include "NovelMind/scripting/script_runtime.hpp"
#include <cstring>

namespace NovelMind::scripting {

ScriptRuntime::ScriptRuntime() = default;
ScriptRuntime::~ScriptRuntime() = default;

Result<void> ScriptRuntime::load(const CompiledScript &script) {
  m_script = script;

  auto result = m_vm.load(script.instructions, script.stringTable);
  if (!result.isOk()) {
    return Result<void>::error(result.error());
  }

  registerCallbacks();
  m_state = RuntimeState::Idle;

  return Result<void>::ok();
}

void ScriptRuntime::setSceneManager(scene::SceneManager *manager) {
  m_sceneManager = manager;
}

void ScriptRuntime::setDialogueBox(Scene::DialogueBox *dialogueBox) {
  m_dialogueBox = dialogueBox;
}

void ScriptRuntime::setChoiceMenu(Scene::ChoiceMenu *choiceMenu) {
  m_choiceMenu = choiceMenu;
}

void ScriptRuntime::setAudioManager(audio::AudioManager *manager) {
  m_audioManager = manager;
}

void ScriptRuntime::setAnimationManager(scene::AnimationManager *manager) {
  m_animationManager = manager;
}

void ScriptRuntime::setConfig(const RuntimeConfig &config) {
  m_config = config;
}

const RuntimeConfig &ScriptRuntime::getConfig() const { return m_config; }

void ScriptRuntime::start() {
  if (m_script.sceneEntryPoints.empty()) {
    m_state = RuntimeState::Halted;
    return;
  }

  // Start from first scene
  auto it = m_script.sceneEntryPoints.begin();
  gotoScene(it->first);
}

Result<void> ScriptRuntime::gotoScene(const std::string &sceneName) {
  auto it = m_script.sceneEntryPoints.find(sceneName);
  if (it == m_script.sceneEntryPoints.end()) {
    return Result<void>::error("Scene not found: " + sceneName);
  }

  m_currentScene = sceneName;
  m_vm.reset();

  // Load the full program and set IP to scene entry
  m_vm.load(m_script.instructions, m_script.stringTable);

  // Set instruction pointer manually would require VM modification
  // For now, we'll run until we reach the scene

  m_state = RuntimeState::Running;
  fireEvent(ScriptEventType::SceneChange, sceneName);

  return Result<void>::ok();
}

void ScriptRuntime::update(f64 deltaTime) {
  switch (m_state) {
  case RuntimeState::Idle:
  case RuntimeState::Halted:
  case RuntimeState::Paused:
    return;

  case RuntimeState::WaitingInput:
    // Check for auto-advance
    if (m_config.autoAdvanceEnabled && m_dialogueBox) {
      // Auto-advance handled by dialogue box
    }
    return;

  case RuntimeState::WaitingChoice:
    // Just wait for selectChoice() call
    return;

  case RuntimeState::WaitingTimer:
    updateWaitTimer(deltaTime);
    return;

  case RuntimeState::WaitingTransition:
    updateTransition(deltaTime);
    return;

  case RuntimeState::WaitingAnimation:
    updateAnimation(deltaTime);
    return;

  case RuntimeState::Running:
    // Execute VM steps
    if (m_skipMode) {
      // In skip mode, run faster
      for (i32 i = 0; i < 10 && m_state == RuntimeState::Running; ++i) {
        if (!m_vm.step()) {
          m_state = RuntimeState::Halted;
          break;
        }

        if (m_vm.isWaiting() || m_vm.isPaused()) {
          break;
        }
      }
    } else {
      // Normal execution
      if (!m_vm.step()) {
        m_state = RuntimeState::Halted;
      }

      if (m_vm.isHalted()) {
        m_state = RuntimeState::Halted;
      }
    }
    break;
  }

  // Update dialogue
  updateDialogue(deltaTime);
}

void ScriptRuntime::continueExecution() {
  if (m_state == RuntimeState::WaitingInput) {
    m_state = RuntimeState::Running;
    m_vm.signalContinue();

    if (m_dialogueBox) {
      m_dialogueBox->clear();
    }
  }
}

void ScriptRuntime::selectChoice(i32 index) {
  if (m_state == RuntimeState::WaitingChoice) {
    if (index >= 0 && index < static_cast<i32>(m_currentChoices.size())) {
      m_selectedChoice = index;
      m_vm.signalChoice(index);
      m_state = RuntimeState::Running;

      fireEvent(ScriptEventType::ChoiceSelected,
                m_currentChoices[static_cast<size_t>(index)],
                Value{static_cast<i32>(index)});

      if (m_choiceMenu) {
        m_choiceMenu->setVisible(false);
      }

      m_currentChoices.clear();
    }
  }
}

void ScriptRuntime::pause() {
  if (m_state == RuntimeState::Running) {
    m_state = RuntimeState::Paused;
    m_vm.pause();
  }
}

void ScriptRuntime::resume() {
  if (m_state == RuntimeState::Paused) {
    m_state = RuntimeState::Running;
    m_vm.resume();
  }
}

void ScriptRuntime::stop() {
  m_state = RuntimeState::Halted;
  m_vm.reset();
}

RuntimeState ScriptRuntime::getState() const { return m_state; }

bool ScriptRuntime::isWaitingForInput() const {
  return m_state == RuntimeState::WaitingInput;
}

bool ScriptRuntime::isWaitingForChoice() const {
  return m_state == RuntimeState::WaitingChoice;
}

bool ScriptRuntime::isComplete() const {
  return m_state == RuntimeState::Halted;
}

const std::string &ScriptRuntime::getCurrentScene() const {
  return m_currentScene;
}

void ScriptRuntime::setVariable(const std::string &name, Value value) {
  m_vm.setVariable(name, value);
  fireEvent(ScriptEventType::VariableChanged, name, value);
}

Value ScriptRuntime::getVariable(const std::string &name) const {
  return m_vm.getVariable(name);
}

void ScriptRuntime::setFlag(const std::string &name, bool value) {
  m_vm.setFlag(name, value);
  fireEvent(ScriptEventType::FlagChanged, name, Value{value});
}

bool ScriptRuntime::getFlag(const std::string &name) const {
  return m_vm.getFlag(name);
}

void ScriptRuntime::setSkipMode(bool enabled) { m_skipMode = enabled; }

bool ScriptRuntime::isSkipMode() const { return m_skipMode; }

RuntimeSaveState ScriptRuntime::saveState() const {
  RuntimeSaveState state;
  state.currentScene = m_currentScene;
  state.instructionPointer = 0; // Would need VM modification to get this

  // Save all variables from VM (would need VM modification)
  state.inDialogue = m_dialogueActive;

  return state;
}

Result<void> ScriptRuntime::loadState(const RuntimeSaveState &state) {
  // Load variables back to VM
  for (const auto &[name, value] : state.variables) {
    m_vm.setVariable(name, value);
  }

  for (const auto &[name, value] : state.flags) {
    m_vm.setFlag(name, value);
  }

  // Go to scene
  if (!state.currentScene.empty()) {
    return gotoScene(state.currentScene);
  }

  return Result<void>::ok();
}

void ScriptRuntime::setEventCallback(EventCallback callback) {
  m_eventCallback = std::move(callback);
}

VirtualMachine &ScriptRuntime::getVM() { return m_vm; }

// VM callback handlers

void ScriptRuntime::onShowBackground(const std::vector<Value> &args) {
  if (args.empty()) {
    return;
  }

  std::string bgName = asString(args[0]);

  // The scene manager would load and display the background
  if (m_sceneManager) {
    // m_sceneManager->setBackground(bgName);
  }

  fireEvent(ScriptEventType::SceneChange, bgName);
}

void ScriptRuntime::onShowCharacter(const std::vector<Value> &args) {
  if (args.size() < 2) {
    return;
  }

  std::string charId = asString(args[0]);
  i32 posCode = asInt(args[1]);
  Scene::CharacterPosition position = parsePosition(posCode);
  (void)position; // Will be used when scene manager integration is implemented

  // Find character definition
  auto it = m_script.characters.find(charId);
  if (it == m_script.characters.end()) {
    return;
  }

  // Create or get character sprite
  if (m_sceneManager) {
    // m_sceneManager->showCharacter(charId, position);
  }

  fireEvent(ScriptEventType::CharacterShow, charId);
}

void ScriptRuntime::onHideCharacter(const std::vector<Value> &args) {
  if (args.empty()) {
    return;
  }

  std::string charId = asString(args[0]);

  if (m_sceneManager) {
    // m_sceneManager->hideCharacter(charId);
  }

  fireEvent(ScriptEventType::CharacterHide, charId);
}

void ScriptRuntime::onSay(const std::vector<Value> &args) {
  if (args.empty()) {
    return;
  }

  std::string text = asString(args[0]);
  std::string speaker;

  if (args.size() > 1 && !isNull(args[1])) {
    speaker = asString(args[1]);
  }

  if (m_dialogueBox) {
    if (!speaker.empty()) {
      // Look up character for name color
      auto it = m_script.characters.find(speaker);
      if (it != m_script.characters.end()) {
        m_dialogueBox->setSpeakerName(it->second.displayName);
      } else {
        m_dialogueBox->setSpeakerName(speaker);
      }
    } else {
      m_dialogueBox->setSpeakerName("");
    }

    f32 speed = m_skipMode ? m_config.skipModeSpeed : m_config.defaultTextSpeed;
    m_dialogueBox->setText(text);
    m_dialogueBox->setTypewriterSpeed(speed);
    m_dialogueBox->startTypewriter();
    m_dialogueBox->show();

    m_dialogueActive = true;
  }

  m_state = RuntimeState::WaitingInput;
  fireEvent(ScriptEventType::DialogueStart, speaker, Value{text});
}

void ScriptRuntime::onChoice(const std::vector<Value> &args) {
  if (args.empty()) {
    return;
  }

  i32 count = asInt(args[0]);
  m_currentChoices.clear();

  // Extract choice texts from the next args
  for (i32 i = 1; i <= count && i < static_cast<i32>(args.size()); ++i) {
    m_currentChoices.push_back(asString(args[static_cast<size_t>(i)]));
  }

  if (m_choiceMenu) {
    m_choiceMenu->clearOptions();
    for (size_t i = 0; i < m_currentChoices.size(); ++i) {
      m_choiceMenu->addOption(m_currentChoices[i]);
    }
    m_choiceMenu->setVisible(true);
  }

  m_state = RuntimeState::WaitingChoice;
  fireEvent(ScriptEventType::ChoiceStart);
}

void ScriptRuntime::onGotoScene(const std::vector<Value> &args) {
  if (args.empty()) {
    return;
  }

  std::string sceneName = asString(args[0]);
  gotoScene(sceneName);
}

void ScriptRuntime::onWait(const std::vector<Value> &args) {
  if (args.empty()) {
    return;
  }

  // Duration is stored as raw bits
  u32 durBits = static_cast<u32>(asInt(args[0]));
  f32 duration;
  std::memcpy(&duration, &durBits, sizeof(f32));

  m_waitTimer = duration;
  m_state = RuntimeState::WaitingTimer;
}

void ScriptRuntime::onPlaySound(const std::vector<Value> &args) {
  if (args.empty()) {
    return;
  }

  std::string soundId = asString(args[0]);

  if (m_audioManager) {
    // m_audioManager->playSound(soundId);
  }

  fireEvent(ScriptEventType::SoundPlay, soundId);
}

void ScriptRuntime::onPlayMusic(const std::vector<Value> &args) {
  if (args.empty()) {
    return;
  }

  std::string musicId = asString(args[0]);

  if (m_audioManager) {
    // m_audioManager->playMusic(musicId);
  }

  fireEvent(ScriptEventType::MusicStart, musicId);
}

void ScriptRuntime::onStopMusic(const std::vector<Value> &args) {
  f32 fadeOut = 0.0f;

  if (!args.empty()) {
    u32 durBits = static_cast<u32>(asInt(args[0]));
    std::memcpy(&fadeOut, &durBits, sizeof(f32));
  }

  if (m_audioManager) {
    // m_audioManager->stopMusic(fadeOut);
  }

  fireEvent(ScriptEventType::MusicStop);
}

void ScriptRuntime::onTransition(const std::vector<Value> &args) {
  if (args.size() < 2) {
    return;
  }

  std::string type = asString(args[0]);
  u32 durBits = static_cast<u32>(asInt(args[1]));
  f32 duration;
  std::memcpy(&duration, &durBits, sizeof(f32));

  m_activeTransition = createTransition(type, duration);
  if (m_activeTransition) {
    m_activeTransition->start(duration);
    m_state = RuntimeState::WaitingTransition;
    fireEvent(ScriptEventType::TransitionStart, type);
  }
}

// Internal helpers

void ScriptRuntime::registerCallbacks() {
  m_vm.registerCallback(OpCode::SHOW_BACKGROUND,
                        [this](const auto &args) { onShowBackground(args); });

  m_vm.registerCallback(OpCode::SHOW_CHARACTER,
                        [this](const auto &args) { onShowCharacter(args); });

  m_vm.registerCallback(OpCode::HIDE_CHARACTER,
                        [this](const auto &args) { onHideCharacter(args); });

  m_vm.registerCallback(OpCode::SAY, [this](const auto &args) { onSay(args); });

  m_vm.registerCallback(OpCode::CHOICE,
                        [this](const auto &args) { onChoice(args); });

  m_vm.registerCallback(OpCode::GOTO_SCENE,
                        [this](const auto &args) { onGotoScene(args); });

  m_vm.registerCallback(OpCode::WAIT,
                        [this](const auto &args) { onWait(args); });

  m_vm.registerCallback(OpCode::PLAY_SOUND,
                        [this](const auto &args) { onPlaySound(args); });

  m_vm.registerCallback(OpCode::PLAY_MUSIC,
                        [this](const auto &args) { onPlayMusic(args); });

  m_vm.registerCallback(OpCode::STOP_MUSIC,
                        [this](const auto &args) { onStopMusic(args); });

  m_vm.registerCallback(OpCode::TRANSITION,
                        [this](const auto &args) { onTransition(args); });
}

void ScriptRuntime::fireEvent(ScriptEventType type, const std::string &name,
                              const Value &value) {
  if (m_eventCallback) {
    ScriptEvent event;
    event.type = type;
    event.name = name;
    event.value = value;
    m_eventCallback(event);
  }
}

void ScriptRuntime::updateWaitTimer(f64 deltaTime) {
  m_waitTimer -= static_cast<f32>(deltaTime);
  if (m_waitTimer <= 0.0f) {
    m_waitTimer = 0.0f;
    m_state = RuntimeState::Running;
  }
}

void ScriptRuntime::updateTransition(f64 deltaTime) {
  if (m_activeTransition) {
    m_activeTransition->update(deltaTime);

    if (m_activeTransition->isComplete()) {
      m_activeTransition.reset();
      m_state = RuntimeState::Running;
      fireEvent(ScriptEventType::TransitionComplete);
    }
  } else {
    m_state = RuntimeState::Running;
  }
}

void ScriptRuntime::updateAnimation(f64 /*deltaTime*/) {
  if (m_animationManager) {
    // Check if any blocking animations are complete
    if (m_animationManager->count() == 0) {
      m_state = RuntimeState::Running;
    }
  } else {
    m_state = RuntimeState::Running;
  }
}

void ScriptRuntime::updateDialogue(f64 deltaTime) {
  if (m_dialogueActive && m_dialogueBox) {
    m_dialogueBox->update(deltaTime);

    if (m_dialogueBox->isTypewriterComplete() &&
        !m_dialogueBox->isWaitingForInput()) {
      fireEvent(ScriptEventType::DialogueComplete);

      // Handle auto-advance
      if (m_config.autoAdvanceEnabled) {
        // Auto advance logic could be added here
      }
    }
  }
}

Scene::CharacterPosition ScriptRuntime::parsePosition(i32 posCode) {
  switch (posCode) {
  case 0:
    return Scene::CharacterPosition::Left;
  case 1:
    return Scene::CharacterPosition::Center;
  case 2:
    return Scene::CharacterPosition::Right;
  default:
    return Scene::CharacterPosition::Custom;
  }
}

std::unique_ptr<Scene::ITransition>
ScriptRuntime::createTransition(const std::string &type, f32 /*duration*/) {
  std::unique_ptr<Scene::ITransition> transition;

  if (type == "fade") {
    transition =
        std::make_unique<Scene::FadeTransition>(renderer::Color::Black, true);
  } else if (type == "slide") {
    transition = std::make_unique<Scene::SlideTransition>(
        Scene::SlideTransition::Direction::Left);
  } else if (type == "dissolve") {
    transition = std::make_unique<Scene::DissolveTransition>();
  } else if (type == "fadethrough") {
    transition =
        std::make_unique<Scene::FadeThroughTransition>(renderer::Color::Black);
  } else {
    // Default to fade
    transition =
        std::make_unique<Scene::FadeTransition>(renderer::Color::Black, true);
  }

  return transition;
}

} // namespace NovelMind::scripting
