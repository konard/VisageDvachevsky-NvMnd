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
    return; // Already playing
  }

  if (m_playMode == Stopped) {
    // Start from beginning
    m_mockStepIndex = 0;
    m_variables["affection"] = 50;
    m_variables["chapter"] = 1;
    m_callStack = {"main::start"};
    startMockRuntime();
  } else if (m_playMode == Paused) {
    // Resume from pause
    m_mockTimer->start();
  }

  m_playMode = Playing;
  emit playModeChanged(Playing);
  qDebug() << "[PlayMode] Started/Resumed playback";
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
  if (m_mockStepIndex >= m_mockNodeSequence.size()) {
    // Reached end of sequence
    qDebug() << "[MockRuntime] Reached end of execution";
    stop();
    return;
  }

  // Execute next node
  m_currentNodeId = m_mockNodeSequence[m_mockStepIndex];
  emit currentNodeChanged(m_currentNodeId);
  qDebug() << "[MockRuntime] Executing node:" << m_currentNodeId;

  // Simulate variable changes
  if (m_currentNodeId.contains("choice")) {
    m_variables["affection"] = m_variables["affection"].toInt() + 10;
  } else if (m_currentNodeId.contains("scene")) {
    m_variables["chapter"] = m_variables["chapter"].toInt() + 1;
    m_variables["currentLocation"] = "ParkBench";
  }

  emit variablesChanged(m_variables);

  // Simulate call stack changes
  if (m_currentNodeId.contains("dialogue")) {
    m_callStack << QString("scene_%1::dialogue_%2")
                       .arg(m_variables["chapter"].toInt())
                       .arg(m_mockStepIndex);
  } else if (m_currentNodeId == "node_end") {
    m_callStack.clear();
    m_callStack << "main::start";
  }
  emit callStackChanged(m_callStack);

  // Check for breakpoint
  checkBreakpoint();

  // Advance to next step
  m_mockStepIndex++;
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
