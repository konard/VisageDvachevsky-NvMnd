/**
 * @file editor_runtime_host.cpp
 * @brief EditorRuntimeHost implementation
 */

#include "NovelMind/editor/editor_runtime_host.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>

namespace NovelMind::editor {

namespace fs = std::filesystem;
namespace {

bool readFileToString(std::ifstream &file, std::string &out) {
  file.seekg(0, std::ios::end);
  const std::streampos size = file.tellg();
  if (size < 0) {
    return false;
  }

  out.resize(static_cast<size_t>(size));
  file.seekg(0, std::ios::beg);
  file.read(out.data(), static_cast<std::streamsize>(out.size()));
  return static_cast<bool>(file);
}

} // namespace

// ============================================================================
// EditorRuntimeHost
// ============================================================================

EditorRuntimeHost::EditorRuntimeHost() {}

EditorRuntimeHost::~EditorRuntimeHost() {
  if (m_state != EditorRuntimeState::Unloaded) {
    stop();
  }
}

Result<void> EditorRuntimeHost::loadProject(const ProjectDescriptor &project) {
  if (m_projectLoaded) {
    unloadProject();
  }

  m_project = project;

  // Validate project paths
  if (!fs::exists(project.path)) {
    return Result<void>::error("Project path does not exist: " + project.path);
  }

  // Set default scripts path if not specified
  if (m_project.scriptsPath.empty()) {
    m_project.scriptsPath = (fs::path(project.path) / "scripts").string();
  }

  // Set default assets path if not specified
  if (m_project.assetsPath.empty()) {
    m_project.assetsPath = (fs::path(project.path) / "assets").string();
  }

  // Compile the project scripts
  auto compileResult = compileProject();
  if (!compileResult.isOk()) {
    return compileResult;
  }

  // Initialize runtime components
  auto initResult = initializeRuntime();
  if (!initResult.isOk()) {
    return initResult;
  }

  m_projectLoaded = true;
  m_state = EditorRuntimeState::Stopped;
  fireStateChanged(m_state);

  return Result<void>::ok();
}

void EditorRuntimeHost::unloadProject() {
  if (m_state == EditorRuntimeState::Running ||
      m_state == EditorRuntimeState::Paused) {
    stop();
  }

  m_scriptRuntime.reset();
  m_sceneGraph.reset();
  m_animationManager.reset();
  m_audioManager.reset();
  m_program.reset();
  m_compiledScript.reset();

  m_sceneNames.clear();
  m_fileTimestamps.clear();

  m_project = ProjectDescriptor();
  m_projectLoaded = false;
  m_state = EditorRuntimeState::Unloaded;
  fireStateChanged(m_state);
}

bool EditorRuntimeHost::isProjectLoaded() const { return m_projectLoaded; }

const ProjectDescriptor &EditorRuntimeHost::getProject() const {
  return m_project;
}

// ============================================================================
// Playback Control
// ============================================================================

Result<void> EditorRuntimeHost::play() {
  if (!m_projectLoaded) {
    return Result<void>::error("No project loaded");
  }

  if (!m_project.startScene.empty()) {
    return playFromScene(m_project.startScene);
  }

  // Find first scene if no start scene specified
  if (!m_sceneNames.empty()) {
    return playFromScene(m_sceneNames[0]);
  }

  return Result<void>::error("No scenes found in project");
}

Result<void> EditorRuntimeHost::playFromScene(const std::string &sceneId) {
  if (!m_projectLoaded) {
    return Result<void>::error("No project loaded");
  }

  // Reset runtime state
  resetRuntime();

  // Load compiled script into runtime
  if (m_compiledScript && m_scriptRuntime) {
    auto loadResult = m_scriptRuntime->load(*m_compiledScript);
    if (!loadResult.isOk()) {
      m_state = EditorRuntimeState::Error;
      fireStateChanged(m_state);
      if (m_onRuntimeError) {
        m_onRuntimeError("Failed to load script: " + loadResult.error());
      }
      return loadResult;
    }

    // Go to the specified scene
    auto gotoResult = m_scriptRuntime->gotoScene(sceneId);
    if (!gotoResult.isOk()) {
      m_state = EditorRuntimeState::Error;
      fireStateChanged(m_state);
      if (m_onRuntimeError) {
        m_onRuntimeError("Failed to go to scene: " + gotoResult.error());
      }
      return gotoResult;
    }
  }

  m_state = EditorRuntimeState::Running;
  fireStateChanged(m_state);

  if (m_onSceneChanged) {
    m_onSceneChanged(sceneId);
  }

  return Result<void>::ok();
}

void EditorRuntimeHost::pause() {
  if (m_state == EditorRuntimeState::Running) {
    m_state = EditorRuntimeState::Paused;
    fireStateChanged(m_state);

    if (m_scriptRuntime) {
      m_scriptRuntime->pause();
    }
  }
}

void EditorRuntimeHost::resume() {
  if (m_state == EditorRuntimeState::Paused) {
    m_state = EditorRuntimeState::Running;
    fireStateChanged(m_state);

    if (m_scriptRuntime) {
      m_scriptRuntime->resume();
    }
  }
}

void EditorRuntimeHost::stop() {
  if (m_state == EditorRuntimeState::Running ||
      m_state == EditorRuntimeState::Paused ||
      m_state == EditorRuntimeState::Stepping) {
    if (m_scriptRuntime) {
      m_scriptRuntime->stop();
    }

    // Reset scene to initial state
    if (m_sceneGraph) {
      m_sceneGraph->clear();
    }

    m_state = EditorRuntimeState::Stopped;
    fireStateChanged(m_state);
  }
}

void EditorRuntimeHost::stepFrame() {
  if (!m_projectLoaded) {
    return;
  }

  if (m_state == EditorRuntimeState::Stopped) {
    // Start in stepping mode
    play();
    m_state = EditorRuntimeState::Stepping;
  }

  if (m_state == EditorRuntimeState::Paused ||
      m_state == EditorRuntimeState::Stepping) {
    // Execute one frame worth of time
    f64 frameTime = 1.0 / 60.0;
    if (m_scriptRuntime) {
      m_scriptRuntime->update(frameTime);
    }
    if (m_sceneGraph) {
      m_sceneGraph->update(frameTime);
    }
    if (m_animationManager) {
      m_animationManager->update(frameTime);
    }
  }
}

void EditorRuntimeHost::stepScriptInstruction() {
  if (!m_projectLoaded || !m_scriptRuntime) {
    return;
  }

  if (m_state == EditorRuntimeState::Stopped) {
    // Start in stepping mode
    play();
  }

  m_state = EditorRuntimeState::Stepping;
  m_singleStepping = true;

  // Execute exactly one VM instruction
  // The VM will pause after the next instruction
  m_scriptRuntime->continueExecution();

  fireStateChanged(m_state);
}

void EditorRuntimeHost::stepLine() {
  if (!m_projectLoaded || !m_scriptRuntime) {
    return;
  }

  // Step until we reach a different source line
  // Implementation would track current line and continue until line changes
  stepScriptInstruction();
}

void EditorRuntimeHost::continueExecution() {
  if (m_state == EditorRuntimeState::Paused ||
      m_state == EditorRuntimeState::Stepping) {
    m_singleStepping = false;
    m_state = EditorRuntimeState::Running;
    fireStateChanged(m_state);

    if (m_scriptRuntime) {
      m_scriptRuntime->resume();
    }
  }
}

EditorRuntimeState EditorRuntimeHost::getState() const { return m_state; }

void EditorRuntimeHost::update(f64 deltaTime) {
  if (m_state != EditorRuntimeState::Running) {
    return;
  }

  // Check for hot reload if enabled
  if (m_autoHotReload) {
    checkForFileChanges();
  }

  // Update script runtime
  if (m_scriptRuntime) {
    m_scriptRuntime->update(deltaTime);

    // Check if runtime completed
    if (m_scriptRuntime->isComplete()) {
      m_state = EditorRuntimeState::Stopped;
      fireStateChanged(m_state);
    }
  }

  // Update scene graph
  if (m_sceneGraph) {
    m_sceneGraph->update(deltaTime);
  }

  // Update animations
  if (m_animationManager) {
    m_animationManager->update(deltaTime);
  }
}

// ============================================================================
// User Input Simulation
// ============================================================================

void EditorRuntimeHost::simulateClick() {
  if (m_scriptRuntime && m_state == EditorRuntimeState::Running) {
    if (m_scriptRuntime->isWaitingForInput()) {
      m_scriptRuntime->continueExecution();
    }
  }
}

void EditorRuntimeHost::simulateChoiceSelect(i32 index) {
  if (m_scriptRuntime && m_state == EditorRuntimeState::Running) {
    if (m_scriptRuntime->isWaitingForChoice()) {
      m_scriptRuntime->selectChoice(index);
    }
  }
}

void EditorRuntimeHost::simulateKeyPress(i32 /*keyCode*/) {
  // Handle keyboard input
  // This would translate key codes to runtime actions
}

// ============================================================================
// Inspection APIs
// ============================================================================

SceneSnapshot EditorRuntimeHost::getSceneSnapshot() const {
  SceneSnapshot snapshot;

  if (!m_sceneGraph) {
    return snapshot;
  }

  snapshot.currentSceneId = m_sceneGraph->getSceneId();

  // Get background
  const auto &bgLayer =
      const_cast<scene::SceneGraph *>(m_sceneGraph.get())->getBackgroundLayer();
  const auto &bgObjects = bgLayer.getObjects();
  if (!bgObjects.empty()) {
    if (auto *bg =
            dynamic_cast<scene::BackgroundObject *>(bgObjects[0].get())) {
      snapshot.activeBackground = bg->getTextureId();
    }
  }

  // Get visible characters
  const auto &charLayer =
      const_cast<scene::SceneGraph *>(m_sceneGraph.get())->getCharacterLayer();
  for (const auto &obj : charLayer.getObjects()) {
    if (obj->isVisible()) {
      snapshot.visibleCharacters.push_back(obj->getId());
      if (auto *charObj = dynamic_cast<scene::CharacterObject *>(obj.get())) {
        snapshot.characterExpressions.emplace_back(charObj->getCharacterId(),
                                                   charObj->getExpression());
      }
    }
  }

  // Get dialogue state
  const auto &uiLayer =
      const_cast<scene::SceneGraph *>(m_sceneGraph.get())->getUILayer();
  for (const auto &obj : uiLayer.getObjects()) {
    if (obj->getType() == scene::SceneObjectType::DialogueUI) {
      if (auto *dlg = dynamic_cast<scene::DialogueUIObject *>(obj.get())) {
        snapshot.dialogueVisible = dlg->isVisible();
        snapshot.dialogueSpeaker = dlg->getSpeaker();
        snapshot.dialogueText = dlg->getText();
      }
    } else if (obj->getType() == scene::SceneObjectType::ChoiceUI) {
      if (auto *choice = dynamic_cast<scene::ChoiceUIObject *>(obj.get())) {
        snapshot.choiceMenuVisible = choice->isVisible();
        for (const auto &opt : choice->getChoices()) {
          snapshot.choiceOptions.push_back(opt.text);
        }
        snapshot.selectedChoice = choice->getSelectedIndex();
      }
    }
  }

  return snapshot;
}

ScriptCallStack EditorRuntimeHost::getScriptCallStack() const {
  ScriptCallStack stack;

  if (!m_scriptRuntime) {
    return stack;
  }

  // Get current execution context from VM
  const auto &vm =
      const_cast<scripting::ScriptRuntime *>(m_scriptRuntime.get())->getVM();

  CallStackEntry entry;
  entry.sceneName = m_scriptRuntime->getCurrentScene();
  entry.instructionPointer = vm.getIP();
  // Source location would be mapped from IP to source

  stack.frames.push_back(entry);
  stack.currentDepth = 1;

  return stack;
}

std::unordered_map<std::string, scripting::Value>
EditorRuntimeHost::getVariables() const {
  std::unordered_map<std::string, scripting::Value> variables;

  if (!m_scriptRuntime) {
    return variables;
  }

  // This would extract variables from the VM state
  // For now, return empty map
  return variables;
}

scripting::Value EditorRuntimeHost::getVariable(const std::string &name) const {
  if (m_scriptRuntime) {
    return m_scriptRuntime->getVariable(name);
  }
  return scripting::Value{};
}

void EditorRuntimeHost::setVariable(const std::string &name,
                                    const scripting::Value &value) {
  if (m_scriptRuntime) {
    m_scriptRuntime->setVariable(name, value);
    if (m_onVariableChanged) {
      m_onVariableChanged(name, value);
    }
  }
}

std::unordered_map<std::string, bool> EditorRuntimeHost::getFlags() const {
  std::unordered_map<std::string, bool> flags;
  // This would extract flags from runtime state
  return flags;
}

bool EditorRuntimeHost::getFlag(const std::string &name) const {
  if (m_scriptRuntime) {
    return m_scriptRuntime->getFlag(name);
  }
  return false;
}

void EditorRuntimeHost::setFlag(const std::string &name, bool value) {
  if (m_scriptRuntime) {
    m_scriptRuntime->setFlag(name, value);
  }
}

std::vector<std::string> EditorRuntimeHost::getScenes() const {
  return m_sceneNames;
}

std::string EditorRuntimeHost::getCurrentScene() const {
  if (m_scriptRuntime) {
    return m_scriptRuntime->getCurrentScene();
  }
  return "";
}

// ============================================================================
// Breakpoints
// ============================================================================

void EditorRuntimeHost::addBreakpoint(const Breakpoint &breakpoint) {
  // Check if breakpoint already exists
  for (auto &bp : m_breakpoints) {
    if (bp.scriptPath == breakpoint.scriptPath && bp.line == breakpoint.line) {
      bp = breakpoint;
      return;
    }
  }
  m_breakpoints.push_back(breakpoint);
}

void EditorRuntimeHost::removeBreakpoint(const std::string &scriptPath,
                                         u32 line) {
  m_breakpoints.erase(std::remove_if(m_breakpoints.begin(), m_breakpoints.end(),
                                     [&](const Breakpoint &bp) {
                                       return bp.scriptPath == scriptPath &&
                                              bp.line == line;
                                     }),
                      m_breakpoints.end());
}

void EditorRuntimeHost::setBreakpointEnabled(const std::string &scriptPath,
                                             u32 line, bool enabled) {
  for (auto &bp : m_breakpoints) {
    if (bp.scriptPath == scriptPath && bp.line == line) {
      bp.enabled = enabled;
      return;
    }
  }
}

const std::vector<Breakpoint> &EditorRuntimeHost::getBreakpoints() const {
  return m_breakpoints;
}

void EditorRuntimeHost::clearBreakpoints() { m_breakpoints.clear(); }

// ============================================================================
// Callbacks
// ============================================================================

void EditorRuntimeHost::setOnStateChanged(OnStateChanged callback) {
  m_onStateChanged = std::move(callback);
}

void EditorRuntimeHost::setOnBreakpointHit(OnBreakpointHit callback) {
  m_onBreakpointHit = std::move(callback);
}

void EditorRuntimeHost::setOnSceneChanged(OnSceneChanged callback) {
  m_onSceneChanged = std::move(callback);
}

void EditorRuntimeHost::setOnVariableChanged(OnVariableChanged callback) {
  m_onVariableChanged = std::move(callback);
}

void EditorRuntimeHost::setOnRuntimeError(OnRuntimeError callback) {
  m_onRuntimeError = std::move(callback);
}

// ============================================================================
// Scene Graph Access
// ============================================================================

scene::SceneGraph *EditorRuntimeHost::getSceneGraph() {
  return m_sceneGraph.get();
}

scripting::ScriptRuntime *EditorRuntimeHost::getScriptRuntime() {
  return m_scriptRuntime.get();
}

// ============================================================================
// Hot Reload
// ============================================================================

Result<void> EditorRuntimeHost::reloadScripts() {
  // Save current state
  scripting::RuntimeSaveState savedState;
  if (m_scriptRuntime) {
    savedState = m_scriptRuntime->saveState();
  }

  // Recompile
  auto compileResult = compileProject();
  if (!compileResult.isOk()) {
    return compileResult;
  }

  // Reload into runtime
  if (m_scriptRuntime && m_compiledScript) {
    auto loadResult = m_scriptRuntime->load(*m_compiledScript);
    if (!loadResult.isOk()) {
      return loadResult;
    }

    // Try to restore state
    auto restoreResult = m_scriptRuntime->loadState(savedState);
    if (!restoreResult.isOk()) {
      // State restore failed, start fresh
      return restoreResult;
    }
  }

  return Result<void>::ok();
}

Result<void> EditorRuntimeHost::reloadAsset(const std::string & /*assetPath*/) {
  // This would reload a specific asset from disk
  // For textures: reload texture data
  // For audio: reload audio buffer
  // For now, return success
  return Result<void>::ok();
}

void EditorRuntimeHost::checkForFileChanges() {
  if (!m_projectLoaded) {
    return;
  }

  try {
    fs::path scriptsPath(m_project.scriptsPath);
    if (!fs::exists(scriptsPath)) {
      return;
    }

    bool changesDetected = false;

    for (const auto &entry : fs::recursive_directory_iterator(scriptsPath)) {
      if (entry.is_regular_file()) {
        std::string ext = entry.path().extension().string();
        if (ext == ".nms" || ext == ".nm") {
          std::string path = entry.path().string();
          u64 modTime =
              static_cast<u64>(std::chrono::duration_cast<std::chrono::seconds>(
                                   entry.last_write_time().time_since_epoch())
                                   .count());

          auto it = m_fileTimestamps.find(path);
          if (it != m_fileTimestamps.end()) {
            if (it->second != modTime) {
              changesDetected = true;
              it->second = modTime;
            }
          } else {
            m_fileTimestamps[path] = modTime;
          }
        }
      }
    }

    if (changesDetected) {
      reloadScripts();
    }
  } catch (const fs::filesystem_error &) {
    // Ignore filesystem errors during hot reload check
  }
}

void EditorRuntimeHost::setAutoHotReload(bool enabled) {
  m_autoHotReload = enabled;
}

bool EditorRuntimeHost::isAutoHotReloadEnabled() const {
  return m_autoHotReload;
}

// ============================================================================
// Private Helpers
// ============================================================================

Result<void> EditorRuntimeHost::compileProject() {
  try {
    fs::path scriptsPath(m_project.scriptsPath);

    if (!fs::exists(scriptsPath)) {
      return Result<void>::error("Scripts path does not exist: " +
                                 m_project.scriptsPath);
    }

    // Collect all script files
    std::string allScripts;
    m_sceneNames.clear();

    for (const auto &entry : fs::recursive_directory_iterator(scriptsPath)) {
      if (entry.is_regular_file()) {
        std::string ext = entry.path().extension().string();
        if (ext == ".nms" || ext == ".nm") {
          std::ifstream file(entry.path());
          if (file) {
            std::string content;
            if (!readFileToString(file, content)) {
              return Result<void>::error("Failed to read script file: " +
                                         entry.path().string());
            }
            allScripts += "\n// File: " + entry.path().string() + "\n";
            allScripts += content;

            // Track file timestamps for hot reload
            u64 modTime = static_cast<u64>(
                std::chrono::duration_cast<std::chrono::seconds>(
                    entry.last_write_time().time_since_epoch())
                    .count());
            m_fileTimestamps[entry.path().string()] = modTime;
          }
        }
      }
    }

    if (allScripts.empty()) {
      return Result<void>::error("No script files found in: " +
                                 m_project.scriptsPath);
    }

    // Lexer
    scripting::Lexer lexer;
    auto tokensResult = lexer.tokenize(allScripts);

    if (!tokensResult.isOk()) {
      return Result<void>::error("Lexer error: " + tokensResult.error());
    }

    // Parser
    scripting::Parser parser;
    auto parseResult = parser.parse(tokensResult.value());

    if (!parseResult.isOk()) {
      return Result<void>::error("Parse error: " + parseResult.error());
    }

    m_program =
        std::make_unique<scripting::Program>(std::move(parseResult.value()));

    // Extract scene names
    for (const auto &scene : m_program->scenes) {
      m_sceneNames.push_back(scene.name);
    }

    // Validator
    scripting::Validator validator;
    auto validationResult = validator.validate(*m_program);

    if (validationResult.hasErrors()) {
      std::string errorMsg = "Validation errors:\n";
      for (const auto &err : validationResult.errors.errors()) {
        errorMsg += "  " + err.format() + "\n";
      }
      return Result<void>::error(errorMsg);
    }

    // Compiler
    scripting::Compiler compiler;
    auto compileResult = compiler.compile(*m_program);

    if (!compileResult.isOk()) {
      return Result<void>::error("Compilation error: " + compileResult.error());
    }

    m_compiledScript = std::make_unique<scripting::CompiledScript>(
        std::move(compileResult.value()));

    return Result<void>::ok();
  } catch (const std::exception &e) {
    return Result<void>::error(std::string("Exception during compilation: ") +
                               e.what());
  }
}

Result<void> EditorRuntimeHost::initializeRuntime() {
  // Create scene graph
  m_sceneGraph = std::make_unique<scene::SceneGraph>();

  // Create animation manager
  m_animationManager = std::make_unique<scene::AnimationManager>();

  // Create audio manager (dev mode - unencrypted)
  m_audioManager = std::make_unique<audio::AudioManager>();

  // Create script runtime
  m_scriptRuntime = std::make_unique<scripting::ScriptRuntime>();

  // Connect runtime to scene components
  // Note: In a full implementation, we would also connect:
  // - SceneManager
  // - DialogueBox
  // - ChoiceMenu
  // - AudioManager

  // Set up event callback
  m_scriptRuntime->setEventCallback(
      [this](const scripting::ScriptEvent &event) { onRuntimeEvent(event); });

  return Result<void>::ok();
}

void EditorRuntimeHost::resetRuntime() {
  if (m_sceneGraph) {
    m_sceneGraph->clear();
  }

  if (m_animationManager) {
    m_animationManager->stopAll();
  }

  m_singleStepping = false;
  m_targetInstructionPointer = 0;
}

bool EditorRuntimeHost::checkBreakpoint(
    const scripting::SourceLocation &location) {
  for (const auto &bp : m_breakpoints) {
    if (bp.enabled && bp.line == location.line) {
      // Check file match (simplified)
      if (bp.condition.empty()) {
        fireBreakpointHit(bp);
        return true;
      }
      // Would evaluate conditional breakpoint here
    }
  }
  return false;
}

void EditorRuntimeHost::fireStateChanged(EditorRuntimeState newState) {
  if (m_onStateChanged) {
    m_onStateChanged(newState);
  }
}

void EditorRuntimeHost::fireBreakpointHit(const Breakpoint &bp) {
  if (m_onBreakpointHit) {
    m_state = EditorRuntimeState::Paused;
    auto stack = getScriptCallStack();
    m_onBreakpointHit(bp, stack);
  }
}

void EditorRuntimeHost::onRuntimeEvent(const scripting::ScriptEvent &event) {
  switch (event.type) {
  case scripting::ScriptEventType::SceneChange:
    if (m_onSceneChanged) {
      m_onSceneChanged(event.name);
    }
    break;

  case scripting::ScriptEventType::VariableChanged:
    if (m_onVariableChanged) {
      m_onVariableChanged(event.name, event.value);
    }
    break;

  default:
    break;
  }
}

} // namespace NovelMind::editor
