#include <NovelMind/editor/qt/nm_play_mode_controller.hpp>
#include <QDebug>
#include <QSettings>

namespace NovelMind::editor::qt {

NMPlayModeController &NMPlayModeController::instance() {
  static NMPlayModeController instance;
  return instance;
}

NMPlayModeController::NMPlayModeController(QObject *parent)
    : QObject(parent), m_mockTimer(new QTimer(this)) {
  // Set up mock runtime timer (1 step per second)
  m_mockTimer->setInterval(1000);
  connect(m_mockTimer, &QTimer::timeout, this,
          &NMPlayModeController::simulateStep);

  // Initialize mock node sequence for demo
  m_mockNodeSequence = {
      "node_start",      "node_dialogue_1",       "node_choice_1",
      "node_dialogue_2", "node_scene_transition", "node_dialogue_3",
      "node_end"};

  // Initialize demo variables
  m_variables["playerName"] = "Alice";
  m_variables["affection"] = 50;
  m_variables["chapter"] = 1;
  m_variables["currentLocation"] = "SchoolCourtyard";

  // Initialize demo call stack
  m_callStack = {"main::start"};
}

// === Playback Control ===

void NMPlayModeController::play() {
  if (m_playMode == Playing) {
    qDebug() << "[PlayMode] Already playing, ignoring play() call";
    return; // Already playing
  }

  if (m_playMode == Stopped) {
    // Start from beginning - reset ALL state
    qDebug() << "[PlayMode] Starting playback from beginning";
    m_mockStepIndex = 0;
    m_currentNodeId.clear();

    // Reset variables to initial state
    m_variables.clear();
    m_variables["playerName"] = "Alice";
    m_variables["affection"] = 50;
    m_variables["chapter"] = 1;
    m_variables["currentLocation"] = "SchoolCourtyard";

    // Reset call stack
    m_callStack.clear();
    m_callStack << "main::start";

    qDebug() << "[PlayMode] State reset: step=" << m_mockStepIndex
             << ", affection=" << m_variables["affection"].toInt()
             << ", chapter=" << m_variables["chapter"].toInt();

    startMockRuntime();
  } else if (m_playMode == Paused) {
    // Resume from pause
    qDebug() << "[PlayMode] Resuming from pause at step" << m_mockStepIndex;
    m_mockTimer->start();
  }

  m_playMode = Playing;
  emit playModeChanged(Playing);
  qDebug() << "[PlayMode] Started/Resumed playback, mode =" << m_playMode;
}

void NMPlayModeController::pause() {
  if (m_playMode != Playing) {
    return; // Not playing
  }

  m_mockTimer->stop();
  m_playMode = Paused;
  emit playModeChanged(Paused);
  qDebug() << "[PlayMode] Paused at node:" << m_currentNodeId;
}

void NMPlayModeController::stop() {
  if (m_playMode == Stopped) {
    return; // Already stopped
  }

  stopMockRuntime();
  m_playMode = Stopped;
  m_currentNodeId.clear();
  m_mockStepIndex = 0;

  emit currentNodeChanged(QString()); // Clear current node
  emit playModeChanged(Stopped);
  qDebug() << "[PlayMode] Stopped playback";
}

void NMPlayModeController::stepForward() {
  if (m_playMode == Stopped) {
    // Start in paused mode
    m_mockStepIndex = 0;
    m_playMode = Paused;
    emit playModeChanged(Paused);
  }

  if (m_playMode == Paused) {
    simulateStep();
  }
}

// === Breakpoint Management ===

void NMPlayModeController::toggleBreakpoint(const QString &nodeId) {
  if (m_breakpoints.contains(nodeId)) {
    m_breakpoints.remove(nodeId);
    qDebug() << "[Breakpoint] Removed from" << nodeId;
  } else {
    m_breakpoints.insert(nodeId);
    qDebug() << "[Breakpoint] Added to" << nodeId;
  }
  emit breakpointsChanged();
}

void NMPlayModeController::setBreakpoint(const QString &nodeId, bool enabled) {
  if (enabled) {
    m_breakpoints.insert(nodeId);
  } else {
    m_breakpoints.remove(nodeId);
  }
  emit breakpointsChanged();
}

bool NMPlayModeController::hasBreakpoint(const QString &nodeId) const {
  return m_breakpoints.contains(nodeId);
}

void NMPlayModeController::clearAllBreakpoints() {
  m_breakpoints.clear();
  emit breakpointsChanged();
  qDebug() << "[Breakpoint] Cleared all breakpoints";
}

// === Variable Inspection ===

void NMPlayModeController::setVariable(const QString &name,
                                       const QVariant &value) {
  if (m_playMode != Paused) {
    qWarning() << "[PlayMode] Cannot set variable while not paused";
    return;
  }

  m_variables[name] = value;
  emit variablesChanged(m_variables);
  qDebug() << "[Variable] Set" << name << "=" << value;
}

// === Persistence ===

void NMPlayModeController::loadBreakpoints(const QString &projectPath) {
  QSettings settings(projectPath + "/.novelmind/breakpoints.ini",
                     QSettings::IniFormat);
  settings.beginGroup("Breakpoints");

  m_breakpoints.clear();
  const QStringList keys = settings.childKeys();
  for (const QString &key : keys) {
    if (settings.value(key).toBool()) {
      m_breakpoints.insert(key);
    }
  }

  settings.endGroup();
  emit breakpointsChanged();
  qDebug() << "[Breakpoint] Loaded" << m_breakpoints.size()
           << "breakpoints from project";
}

void NMPlayModeController::saveBreakpoints(const QString &projectPath) {
  QSettings settings(projectPath + "/.novelmind/breakpoints.ini",
                     QSettings::IniFormat);
  settings.beginGroup("Breakpoints");

  settings.remove(""); // Clear all existing
  for (const QString &nodeId : m_breakpoints) {
    settings.setValue(nodeId, true);
  }

  settings.endGroup();
  qDebug() << "[Breakpoint] Saved" << m_breakpoints.size()
           << "breakpoints to project";
}

// === Mock Runtime (Phase 5.0) ===

void NMPlayModeController::startMockRuntime() {
  qDebug() << "[MockRuntime] Starting simulation";
  m_mockTimer->start();

  // Execute first step immediately
  simulateStep();
}

void NMPlayModeController::stopMockRuntime() {
  m_mockTimer->stop();
  qDebug() << "[MockRuntime] Stopped simulation";
}

void NMPlayModeController::simulateStep() {
  // Safety check: ensure we have a valid sequence
  if (m_mockNodeSequence.isEmpty()) {
    qWarning() << "[MockRuntime] ERROR: Node sequence is empty!";
    stop();
    return;
  }

  if (m_mockStepIndex < 0 || m_mockStepIndex >= m_mockNodeSequence.size()) {
    // Reached end of sequence or invalid index
    qDebug() << "[MockRuntime] Reached end of execution (step" << m_mockStepIndex
             << "of" << m_mockNodeSequence.size() << ")";
    stop();
    return;
  }

  // Execute next node with logging
  m_currentNodeId = m_mockNodeSequence[m_mockStepIndex];
  qDebug() << "[MockRuntime] Executing node:" << m_currentNodeId << "(step"
           << m_mockStepIndex << ")";

  // Emit signal BEFORE modifying variables to avoid race conditions
  emit currentNodeChanged(m_currentNodeId);

  // Simulate variable changes with defensive checks
  if (m_currentNodeId.contains("choice")) {
    // Get current affection value safely
    int currentAffection = m_variables.value("affection", 50).toInt();
    m_variables["affection"] = currentAffection + 10;
    qDebug() << "[MockRuntime] Choice node: affection" << currentAffection
             << "->" << (currentAffection + 10);
  } else if (m_currentNodeId.contains("scene")) {
    int currentChapter = m_variables.value("chapter", 1).toInt();
    m_variables["chapter"] = currentChapter + 1;
    m_variables["currentLocation"] = "ParkBench";
    qDebug() << "[MockRuntime] Scene transition: chapter" << currentChapter
             << "->" << (currentChapter + 1);
  }

  emit variablesChanged(m_variables);

  // Simulate call stack changes with safety checks
  if (m_currentNodeId.contains("dialogue")) {
    int currentChapter = m_variables.value("chapter", 1).toInt();
    QString stackEntry =
        QString("scene_%1::dialogue_%2").arg(currentChapter).arg(m_mockStepIndex);
    m_callStack << stackEntry;
    qDebug() << "[MockRuntime] Call stack push:" << stackEntry;
  } else if (m_currentNodeId == "node_end") {
    qDebug() << "[MockRuntime] Reached end node, resetting call stack";
    m_callStack.clear();
    m_callStack << "main::start";
  }
  emit callStackChanged(m_callStack);

  // Check for breakpoint
  checkBreakpoint();

  // Advance to next step
  m_mockStepIndex++;
  qDebug() << "[MockRuntime] Advanced to step" << m_mockStepIndex;
}

void NMPlayModeController::checkBreakpoint() {
  if (m_breakpoints.contains(m_currentNodeId)) {
    qDebug() << "[Breakpoint] Hit at node:" << m_currentNodeId;
    m_mockTimer->stop();
    m_playMode = Paused;
    emit breakpointHit(m_currentNodeId);
    emit playModeChanged(Paused);
  }
}

} // namespace NovelMind::editor::qt
