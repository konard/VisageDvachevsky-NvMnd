#include <NovelMind/editor/qt/nm_icon_manager.hpp>
#include <NovelMind/editor/qt/panels/nm_play_toolbar_panel.hpp>
#include <QToolBar>
#include <QVBoxLayout>

namespace NovelMind::editor::qt {

NMPlayToolbarPanel::NMPlayToolbarPanel(QWidget *parent)
    : NMDockPanel("Play Controls", parent) {
  setupUI();
}

void NMPlayToolbarPanel::onInitialize() {
  NMDockPanel::onInitialize();

  auto &controller = NMPlayModeController::instance();

  // Connect to controller signals
  connect(&controller, &NMPlayModeController::playModeChanged, this,
          &NMPlayToolbarPanel::onPlayModeChanged);
  connect(&controller, &NMPlayModeController::currentNodeChanged, this,
          &NMPlayToolbarPanel::onCurrentNodeChanged);
  connect(&controller, &NMPlayModeController::breakpointHit, this,
          &NMPlayToolbarPanel::onBreakpointHit);

  updateButtonStates();
}

void NMPlayToolbarPanel::onShutdown() {
  NMDockPanel::onShutdown();

  // Stop playback on shutdown
  if (m_currentMode != NMPlayModeController::Stopped) {
    NMPlayModeController::instance().stop();
  }
}

void NMPlayToolbarPanel::onUpdate(double deltaTime) {
  NMDockPanel::onUpdate(deltaTime);
  // Real-time updates if needed
}

void NMPlayToolbarPanel::setupUI() {
  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(4, 4, 4, 4);
  layout->setSpacing(4);

  // Create toolbar
  auto *toolbar = new QToolBar;
  toolbar->setIconSize(QSize(24, 24));
  toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

  auto &iconMgr = NMIconManager::instance();

  // Play button
  m_playButton = new QPushButton;
  m_playButton->setIcon(iconMgr.getIcon("play", 24));
  m_playButton->setText("Play");
  m_playButton->setToolTip("Start or resume playback (F5)");
  m_playButton->setShortcut(Qt::Key_F5);
  m_playButton->setFlat(true);
  connect(m_playButton, &QPushButton::clicked,
          []() { NMPlayModeController::instance().play(); });

  // Pause button
  m_pauseButton = new QPushButton;
  m_pauseButton->setIcon(iconMgr.getIcon("pause", 24));
  m_pauseButton->setText("Pause");
  m_pauseButton->setToolTip("Pause playback");
  m_pauseButton->setFlat(true);
  connect(m_pauseButton, &QPushButton::clicked,
          []() { NMPlayModeController::instance().pause(); });

  // Stop button
  m_stopButton = new QPushButton;
  m_stopButton->setIcon(iconMgr.getIcon("stop", 24));
  m_stopButton->setText("Stop");
  m_stopButton->setToolTip("Stop playback and reset (Shift+F5)");
  m_stopButton->setShortcut(Qt::SHIFT | Qt::Key_F5);
  m_stopButton->setFlat(true);
  connect(m_stopButton, &QPushButton::clicked,
          []() { NMPlayModeController::instance().stop(); });

  // Step Forward button
  m_stepButton = new QPushButton;
  m_stepButton->setIcon(iconMgr.getIcon("step-forward", 24));
  m_stepButton->setText("Step");
  m_stepButton->setToolTip("Execute one instruction (F10)");
  m_stepButton->setShortcut(Qt::Key_F10);
  m_stepButton->setFlat(true);
  connect(m_stepButton, &QPushButton::clicked,
          []() { NMPlayModeController::instance().stepForward(); });

  // Add buttons to toolbar
  toolbar->addWidget(m_playButton);
  toolbar->addWidget(m_pauseButton);
  toolbar->addWidget(m_stopButton);
  toolbar->addSeparator();
  toolbar->addWidget(m_stepButton);
  toolbar->addSeparator();

  // Status label
  m_statusLabel = new QLabel("Stopped");
  m_statusLabel->setStyleSheet("QLabel { color: #a0a0a0; padding: 4px 8px; }");
  toolbar->addWidget(m_statusLabel);

  layout->addWidget(toolbar);
  layout->addStretch();

  setLayout(layout);
}

void NMPlayToolbarPanel::updateButtonStates() {
  const bool isPlaying = (m_currentMode == NMPlayModeController::Playing);
  const bool isPaused = (m_currentMode == NMPlayModeController::Paused);
  const bool isStopped = (m_currentMode == NMPlayModeController::Stopped);

  m_playButton->setEnabled(!isPlaying);
  m_pauseButton->setEnabled(isPlaying);
  m_stopButton->setEnabled(!isStopped);
  m_stepButton->setEnabled(isStopped || isPaused);
}

void NMPlayToolbarPanel::updateStatusLabel() {
  QString status;
  QString color;

  switch (m_currentMode) {
  case NMPlayModeController::Playing:
    status = "Playing";
    color = "#4caf50"; // Green
    if (!m_currentNodeId.isEmpty()) {
      status += QString(" - %1").arg(m_currentNodeId);
    }
    break;
  case NMPlayModeController::Paused:
    status = "Paused";
    color = "#ff9800"; // Orange
    if (!m_currentNodeId.isEmpty()) {
      status += QString(" at %1").arg(m_currentNodeId);
    }
    break;
  case NMPlayModeController::Stopped:
  default:
    status = "Stopped";
    color = "#a0a0a0"; // Gray
    break;
  }

  m_statusLabel->setText(status);
  m_statusLabel->setStyleSheet(
      QString("QLabel { color: %1; padding: 4px 8px; font-weight: bold; }")
          .arg(color));
}

void NMPlayToolbarPanel::onPlayModeChanged(
    NMPlayModeController::PlayMode mode) {
  m_currentMode = mode;
  updateButtonStates();
  updateStatusLabel();
}

void NMPlayToolbarPanel::onCurrentNodeChanged(const QString &nodeId) {
  m_currentNodeId = nodeId;
  updateStatusLabel();
}

void NMPlayToolbarPanel::onBreakpointHit(const QString &nodeId) {
  // Visual feedback for breakpoint hit
  m_statusLabel->setText(QString("⏸️ Breakpoint at %1").arg(nodeId));
  m_statusLabel->setStyleSheet(
      "QLabel { color: #f44336; padding: 4px 8px; font-weight: bold; }"); // Red
}

} // namespace NovelMind::editor::qt
