#include "NovelMind/editor/qt/nm_main_window.hpp"
#include "NovelMind/editor/qt/nm_dock_panel.hpp"
#include "NovelMind/editor/qt/nm_icon_manager.hpp"
#include "NovelMind/editor/qt/nm_style_manager.hpp"
#include "NovelMind/editor/qt/nm_undo_manager.hpp"
#include "NovelMind/editor/qt/panels/nm_asset_browser_panel.hpp"
#include "NovelMind/editor/qt/panels/nm_console_panel.hpp"
#include "NovelMind/editor/qt/panels/nm_debug_overlay_panel.hpp"
#include "NovelMind/editor/qt/panels/nm_hierarchy_panel.hpp"
#include "NovelMind/editor/qt/panels/nm_inspector_panel.hpp"
#include "NovelMind/editor/qt/panels/nm_play_toolbar_panel.hpp"
#include "NovelMind/editor/qt/panels/nm_scene_view_panel.hpp"
#include "NovelMind/editor/qt/panels/nm_story_graph_panel.hpp"

#include <QAction>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QStatusBar>
#include <QToolBar>
#include <QUrl>

namespace NovelMind::editor::qt {

NMMainWindow::NMMainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("NovelMind Editor");
  setMinimumSize(1280, 720);
  resize(1920, 1080);

  // Enable docking with nesting and grouping
  setDockNestingEnabled(true);
  setDockOptions(AnimatedDocks | AllowNestedDocks | AllowTabbedDocks |
                 GroupedDragging);
}

NMMainWindow::~NMMainWindow() { shutdown(); }

bool NMMainWindow::initialize() {
  if (m_initialized)
    return true;

  // Initialize undo/redo system
  NMUndoManager::instance().initialize();

  setupMenuBar();
  setupToolBar();
  setupStatusBar();
  setupPanels();
  setupConnections();
  setupShortcuts();

  // Restore layout or use default
  QSettings settings("NovelMind", "Editor");
  if (settings.contains("mainwindow/geometry")) {
    restoreLayout();
  } else {
    createDefaultLayout();
  }

  // Start update timer
  m_updateTimer = new QTimer(this);
  connect(m_updateTimer, &QTimer::timeout, this, &NMMainWindow::onUpdateTick);
  m_updateTimer->start(UPDATE_INTERVAL_MS);

  m_initialized = true;
  return true;
}

void NMMainWindow::shutdown() {
  if (!m_initialized)
    return;

  if (m_updateTimer) {
    m_updateTimer->stop();
  }

  saveLayout();

  m_initialized = false;
}

void NMMainWindow::setupMenuBar() {
  QMenuBar *menuBar = this->menuBar();
  auto &iconMgr = NMIconManager::instance();

  // =========================================================================
  // File Menu
  // =========================================================================
  QMenu *fileMenu = menuBar->addMenu(tr("&File"));

  m_actionNewProject = fileMenu->addAction(iconMgr.getIcon("file-new", 16),
                                           tr("&New Project..."));
  m_actionNewProject->setShortcut(
      QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));
  m_actionNewProject->setToolTip(tr("Create a new NovelMind project"));

  m_actionOpenProject = fileMenu->addAction(iconMgr.getIcon("file-open", 16),
                                            tr("&Open Project..."));
  m_actionOpenProject->setShortcut(QKeySequence::Open);
  m_actionOpenProject->setToolTip(tr("Open an existing project"));

  fileMenu->addSeparator();

  m_actionSaveProject = fileMenu->addAction(iconMgr.getIcon("file-save", 16),
                                            tr("&Save Project"));
  m_actionSaveProject->setShortcut(QKeySequence::Save);
  m_actionSaveProject->setToolTip(tr("Save the current project"));

  m_actionSaveProjectAs = fileMenu->addAction(iconMgr.getIcon("file-save", 16),
                                              tr("Save Project &As..."));
  m_actionSaveProjectAs->setShortcut(QKeySequence::SaveAs);
  m_actionSaveProjectAs->setToolTip(tr("Save the project with a new name"));

  fileMenu->addSeparator();

  m_actionCloseProject = fileMenu->addAction(iconMgr.getIcon("file-close", 16),
                                             tr("&Close Project"));
  m_actionCloseProject->setToolTip(tr("Close the current project"));

  fileMenu->addSeparator();

  m_actionExit = fileMenu->addAction(tr("E&xit"));
  m_actionExit->setShortcut(QKeySequence::Quit);
  m_actionExit->setToolTip(tr("Exit the editor"));

  // =========================================================================
  // Edit Menu
  // =========================================================================
  QMenu *editMenu = menuBar->addMenu(tr("&Edit"));

  m_actionUndo =
      editMenu->addAction(iconMgr.getIcon("edit-undo", 16), tr("&Undo"));
  m_actionUndo->setShortcut(QKeySequence::Undo);
  m_actionUndo->setToolTip(tr("Undo the last action"));

  m_actionRedo =
      editMenu->addAction(iconMgr.getIcon("edit-redo", 16), tr("&Redo"));
  m_actionRedo->setShortcut(QKeySequence::Redo);
  m_actionRedo->setToolTip(tr("Redo the previously undone action"));

  editMenu->addSeparator();

  m_actionCut =
      editMenu->addAction(iconMgr.getIcon("edit-cut", 16), tr("Cu&t"));
  m_actionCut->setShortcut(QKeySequence::Cut);
  m_actionCut->setToolTip(tr("Cut selected items"));

  m_actionCopy =
      editMenu->addAction(iconMgr.getIcon("edit-copy", 16), tr("&Copy"));
  m_actionCopy->setShortcut(QKeySequence::Copy);
  m_actionCopy->setToolTip(tr("Copy selected items"));

  m_actionPaste =
      editMenu->addAction(iconMgr.getIcon("edit-paste", 16), tr("&Paste"));
  m_actionPaste->setShortcut(QKeySequence::Paste);
  m_actionPaste->setToolTip(tr("Paste from clipboard"));

  m_actionDelete =
      editMenu->addAction(iconMgr.getIcon("edit-delete", 16), tr("&Delete"));
  m_actionDelete->setShortcut(QKeySequence::Delete);
  m_actionDelete->setToolTip(tr("Delete selected items"));

  editMenu->addSeparator();

  m_actionSelectAll = editMenu->addAction(tr("Select &All"));
  m_actionSelectAll->setShortcut(QKeySequence::SelectAll);
  m_actionSelectAll->setToolTip(tr("Select all items"));

  editMenu->addSeparator();

  m_actionPreferences = editMenu->addAction(iconMgr.getIcon("settings", 16),
                                            tr("&Preferences..."));
  m_actionPreferences->setToolTip(tr("Open editor preferences"));

  // =========================================================================
  // View Menu
  // =========================================================================
  QMenu *viewMenu = menuBar->addMenu(tr("&View"));

  QMenu *panelsMenu = viewMenu->addMenu(tr("&Panels"));

  m_actionToggleSceneView = panelsMenu->addAction(
      iconMgr.getIcon("panel-scene", 16), tr("&Scene View"));
  m_actionToggleSceneView->setCheckable(true);
  m_actionToggleSceneView->setChecked(true);
  m_actionToggleSceneView->setToolTip(tr("Toggle Scene View panel"));

  m_actionToggleStoryGraph = panelsMenu->addAction(
      iconMgr.getIcon("panel-graph", 16), tr("Story &Graph"));
  m_actionToggleStoryGraph->setCheckable(true);
  m_actionToggleStoryGraph->setChecked(true);
  m_actionToggleStoryGraph->setToolTip(tr("Toggle Story Graph panel"));

  m_actionToggleInspector = panelsMenu->addAction(
      iconMgr.getIcon("panel-inspector", 16), tr("&Inspector"));
  m_actionToggleInspector->setCheckable(true);
  m_actionToggleInspector->setChecked(true);
  m_actionToggleInspector->setToolTip(tr("Toggle Inspector panel"));

  m_actionToggleConsole = panelsMenu->addAction(
      iconMgr.getIcon("panel-console", 16), tr("&Console"));
  m_actionToggleConsole->setCheckable(true);
  m_actionToggleConsole->setChecked(true);
  m_actionToggleConsole->setToolTip(tr("Toggle Console panel"));

  m_actionToggleAssetBrowser = panelsMenu->addAction(
      iconMgr.getIcon("panel-assets", 16), tr("&Asset Browser"));
  m_actionToggleAssetBrowser->setCheckable(true);
  m_actionToggleAssetBrowser->setChecked(true);
  m_actionToggleAssetBrowser->setToolTip(tr("Toggle Asset Browser panel"));

  m_actionToggleHierarchy = panelsMenu->addAction(
      iconMgr.getIcon("panel-hierarchy", 16), tr("&Hierarchy"));
  m_actionToggleHierarchy->setCheckable(true);
  m_actionToggleHierarchy->setChecked(true);
  m_actionToggleHierarchy->setToolTip(tr("Toggle Hierarchy panel"));

  viewMenu->addSeparator();

  m_actionResetLayout =
      viewMenu->addAction(iconMgr.getIcon("refresh", 16), tr("&Reset Layout"));
  m_actionResetLayout->setToolTip(tr("Reset all panels to default layout"));

  // =========================================================================
  // Play Menu
  // =========================================================================
  QMenu *playMenu = menuBar->addMenu(tr("&Play"));

  m_actionPlay = playMenu->addAction(iconMgr.getIcon("play", 16), tr("&Play"));
  m_actionPlay->setShortcut(Qt::Key_F5);
  m_actionPlay->setToolTip(tr("Start playback (F5)"));

  m_actionPause =
      playMenu->addAction(iconMgr.getIcon("pause", 16), tr("Pa&use"));
  m_actionPause->setShortcut(Qt::Key_F6);
  m_actionPause->setEnabled(false);
  m_actionPause->setToolTip(tr("Pause playback (F6)"));

  m_actionStop = playMenu->addAction(iconMgr.getIcon("stop", 16), tr("&Stop"));
  m_actionStop->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F5));
  m_actionStop->setEnabled(false);
  m_actionStop->setToolTip(tr("Stop playback (Shift+F5)"));

  playMenu->addSeparator();

  m_actionStepFrame = playMenu->addAction(iconMgr.getIcon("step-forward", 16),
                                          tr("Step &Frame"));
  m_actionStepFrame->setShortcut(Qt::Key_F10);
  m_actionStepFrame->setEnabled(false);
  m_actionStepFrame->setToolTip(tr("Step one frame forward (F10)"));

  // =========================================================================
  // Help Menu
  // =========================================================================
  QMenu *helpMenu = menuBar->addMenu(tr("&Help"));

  m_actionDocumentation =
      helpMenu->addAction(iconMgr.getIcon("help", 16), tr("&Documentation"));
  m_actionDocumentation->setShortcut(Qt::Key_F1);
  m_actionDocumentation->setToolTip(tr("Open documentation (F1)"));

  helpMenu->addSeparator();

  m_actionAbout = helpMenu->addAction(iconMgr.getIcon("info", 16),
                                      tr("&About NovelMind Editor..."));
  m_actionAbout->setToolTip(tr("About NovelMind Editor"));
}

void NMMainWindow::setupToolBar() {
  m_mainToolBar = addToolBar(tr("Main Toolbar"));
  m_mainToolBar->setObjectName("MainToolBar");
  m_mainToolBar->setMovable(false);

  // File operations
  m_mainToolBar->addAction(m_actionNewProject);
  m_mainToolBar->addAction(m_actionOpenProject);
  m_mainToolBar->addAction(m_actionSaveProject);

  m_mainToolBar->addSeparator();

  // Edit operations
  m_mainToolBar->addAction(m_actionUndo);
  m_mainToolBar->addAction(m_actionRedo);

  m_mainToolBar->addSeparator();

  // Play controls
  m_mainToolBar->addAction(m_actionPlay);
  m_mainToolBar->addAction(m_actionPause);
  m_mainToolBar->addAction(m_actionStop);
}

void NMMainWindow::setupStatusBar() {
  QStatusBar *status = statusBar();

  m_statusLabel = new QLabel(tr("Ready"));
  status->addWidget(m_statusLabel);
}

void NMMainWindow::setupPanels() {
  auto &iconMgr = NMIconManager::instance();

  // Create all panels with their respective icons
  m_sceneViewPanel = new NMSceneViewPanel(this);
  m_sceneViewPanel->setObjectName("SceneViewPanel");
  m_sceneViewPanel->setWindowIcon(iconMgr.getIcon("panel-scene", 16));

  m_storyGraphPanel = new NMStoryGraphPanel(this);
  m_storyGraphPanel->setObjectName("StoryGraphPanel");
  m_storyGraphPanel->setWindowIcon(iconMgr.getIcon("panel-graph", 16));

  m_inspectorPanel = new NMInspectorPanel(this);
  m_inspectorPanel->setObjectName("InspectorPanel");
  m_inspectorPanel->setWindowIcon(iconMgr.getIcon("panel-inspector", 16));

  m_consolePanel = new NMConsolePanel(this);
  m_consolePanel->setObjectName("ConsolePanel");
  m_consolePanel->setWindowIcon(iconMgr.getIcon("panel-console", 16));

  m_assetBrowserPanel = new NMAssetBrowserPanel(this);
  m_assetBrowserPanel->setObjectName("AssetBrowserPanel");
  m_assetBrowserPanel->setWindowIcon(iconMgr.getIcon("panel-assets", 16));

  m_hierarchyPanel = new NMHierarchyPanel(this);
  m_hierarchyPanel->setObjectName("HierarchyPanel");
  m_hierarchyPanel->setWindowIcon(iconMgr.getIcon("panel-hierarchy", 16));

  // Phase 5 - Play-In-Editor panels
  m_playToolbarPanel = new NMPlayToolbarPanel(this);
  m_playToolbarPanel->setObjectName("PlayToolbarPanel");
  m_playToolbarPanel->setWindowIcon(iconMgr.getIcon("play", 16));

  m_debugOverlayPanel = new NMDebugOverlayPanel(this);
  m_debugOverlayPanel->setObjectName("DebugOverlayPanel");
  m_debugOverlayPanel->setWindowIcon(iconMgr.getIcon("panel-diagnostics", 16));

  // Add panels to the main window
  addDockWidget(Qt::LeftDockWidgetArea, m_hierarchyPanel);
  addDockWidget(Qt::RightDockWidgetArea, m_inspectorPanel);
  addDockWidget(Qt::RightDockWidgetArea, m_debugOverlayPanel); // Phase 5
  addDockWidget(Qt::BottomDockWidgetArea, m_consolePanel);
  addDockWidget(Qt::BottomDockWidgetArea, m_assetBrowserPanel);
  addDockWidget(Qt::TopDockWidgetArea, m_playToolbarPanel); // Phase 5

  // Central area: Scene View and Story Graph as tabs
  setCentralWidget(nullptr);
  addDockWidget(Qt::TopDockWidgetArea, m_sceneViewPanel);
  addDockWidget(Qt::TopDockWidgetArea, m_storyGraphPanel);
  tabifyDockWidget(m_sceneViewPanel, m_storyGraphPanel);
  m_sceneViewPanel->raise(); // Make Scene View the active tab

  // Tab the bottom panels
  tabifyDockWidget(m_consolePanel, m_assetBrowserPanel);
  m_consolePanel->raise();

  // Tab debug overlay with inspector (Phase 5)
  tabifyDockWidget(m_inspectorPanel, m_debugOverlayPanel);
  m_inspectorPanel->raise();
}

void NMMainWindow::setupConnections() {
  // File menu
  connect(m_actionNewProject, &QAction::triggered, this,
          &NMMainWindow::newProjectRequested);
  connect(m_actionOpenProject, &QAction::triggered, this,
          &NMMainWindow::openProjectRequested);
  connect(m_actionSaveProject, &QAction::triggered, this,
          &NMMainWindow::saveProjectRequested);
  connect(m_actionExit, &QAction::triggered, this, &QMainWindow::close);

  // Edit menu - connect to undo manager
  connect(m_actionUndo, &QAction::triggered, &NMUndoManager::instance(),
          &NMUndoManager::undo);
  connect(m_actionRedo, &QAction::triggered, &NMUndoManager::instance(),
          &NMUndoManager::redo);

  // Update undo/redo action states based on undo manager
  connect(&NMUndoManager::instance(), &NMUndoManager::canUndoChanged,
          m_actionUndo, &QAction::setEnabled);
  connect(&NMUndoManager::instance(), &NMUndoManager::canRedoChanged,
          m_actionRedo, &QAction::setEnabled);
  connect(&NMUndoManager::instance(), &NMUndoManager::undoTextChanged,
          [this](const QString &text) {
            m_actionUndo->setText(text.isEmpty() ? tr("&Undo")
                                                 : tr("&Undo %1").arg(text));
          });
  connect(&NMUndoManager::instance(), &NMUndoManager::redoTextChanged,
          [this](const QString &text) {
            m_actionRedo->setText(text.isEmpty() ? tr("&Redo")
                                                 : tr("&Redo %1").arg(text));
          });

  // Initialize undo/redo states
  m_actionUndo->setEnabled(NMUndoManager::instance().canUndo());
  m_actionRedo->setEnabled(NMUndoManager::instance().canRedo());

  // View menu - panel toggles
  connect(m_actionToggleSceneView, &QAction::toggled, m_sceneViewPanel,
          &QDockWidget::setVisible);
  connect(m_actionToggleStoryGraph, &QAction::toggled, m_storyGraphPanel,
          &QDockWidget::setVisible);
  connect(m_actionToggleInspector, &QAction::toggled, m_inspectorPanel,
          &QDockWidget::setVisible);
  connect(m_actionToggleConsole, &QAction::toggled, m_consolePanel,
          &QDockWidget::setVisible);
  connect(m_actionToggleAssetBrowser, &QAction::toggled, m_assetBrowserPanel,
          &QDockWidget::setVisible);
  connect(m_actionToggleHierarchy, &QAction::toggled, m_hierarchyPanel,
          &QDockWidget::setVisible);

  // Sync panel visibility with menu actions
  connect(m_sceneViewPanel, &QDockWidget::visibilityChanged,
          m_actionToggleSceneView, &QAction::setChecked);
  connect(m_storyGraphPanel, &QDockWidget::visibilityChanged,
          m_actionToggleStoryGraph, &QAction::setChecked);
  connect(m_inspectorPanel, &QDockWidget::visibilityChanged,
          m_actionToggleInspector, &QAction::setChecked);
  connect(m_consolePanel, &QDockWidget::visibilityChanged,
          m_actionToggleConsole, &QAction::setChecked);
  connect(m_assetBrowserPanel, &QDockWidget::visibilityChanged,
          m_actionToggleAssetBrowser, &QAction::setChecked);
  connect(m_hierarchyPanel, &QDockWidget::visibilityChanged,
          m_actionToggleHierarchy, &QAction::setChecked);

  connect(m_actionResetLayout, &QAction::triggered, this,
          &NMMainWindow::resetToDefaultLayout);

  // Play menu
  connect(m_actionPlay, &QAction::triggered, this,
          &NMMainWindow::playRequested);
  connect(m_actionStop, &QAction::triggered, this,
          &NMMainWindow::stopRequested);

  // Help menu
  connect(m_actionAbout, &QAction::triggered, this,
          &NMMainWindow::showAboutDialog);
  connect(m_actionDocumentation, &QAction::triggered, []() {
    QDesktopServices::openUrl(QUrl("https://github.com/VisageDvachevsky/NM-"));
  });
}

void NMMainWindow::setupShortcuts() {
  // Shortcuts are already set on the actions in setupMenuBar()
  // This method can be used for additional context-specific shortcuts
}

void NMMainWindow::createDefaultLayout() {
  // Reset all panels to visible
  m_sceneViewPanel->show();
  m_storyGraphPanel->show();
  m_inspectorPanel->show();
  m_consolePanel->show();
  m_assetBrowserPanel->show();
  m_hierarchyPanel->show();

  // Resize to reasonable proportions
  resizeDocks({m_hierarchyPanel}, {250}, Qt::Horizontal);
  resizeDocks({m_inspectorPanel}, {300}, Qt::Horizontal);
  resizeDocks({m_consolePanel, m_assetBrowserPanel}, {200, 200}, Qt::Vertical);
}

void NMMainWindow::onUpdateTick() {
  // Calculate delta time
  static auto lastTime = std::chrono::steady_clock::now();
  auto currentTime = std::chrono::steady_clock::now();
  double deltaTime =
      std::chrono::duration<double>(currentTime - lastTime).count();
  lastTime = currentTime;

  // Update all panels
  if (m_sceneViewPanel)
    m_sceneViewPanel->onUpdate(deltaTime);
  if (m_storyGraphPanel)
    m_storyGraphPanel->onUpdate(deltaTime);
  if (m_inspectorPanel)
    m_inspectorPanel->onUpdate(deltaTime);
  if (m_consolePanel)
    m_consolePanel->onUpdate(deltaTime);
  if (m_assetBrowserPanel)
    m_assetBrowserPanel->onUpdate(deltaTime);
  if (m_hierarchyPanel)
    m_hierarchyPanel->onUpdate(deltaTime);
}

void NMMainWindow::showAboutDialog() {
  QMessageBox::about(this, tr("About NovelMind Editor"),
                     tr("<h3>NovelMind Editor</h3>"
                        "<p>Version 0.3.0</p>"
                        "<p>A modern visual novel editor built with Qt 6.</p>"
                        "<p>Copyright (c) 2024 NovelMind Contributors</p>"
                        "<p>Licensed under MIT License</p>"));
}

void NMMainWindow::togglePanel(NMDockPanel *panel) {
  if (panel) {
    panel->setVisible(!panel->isVisible());
  }
}

void NMMainWindow::setStatusMessage(const QString &message, int timeout) {
  if (m_statusLabel) {
    m_statusLabel->setText(message);
  }
  if (timeout > 0) {
    statusBar()->showMessage(message, timeout);
  }
}

void NMMainWindow::updateWindowTitle(const QString &projectName) {
  if (projectName.isEmpty()) {
    setWindowTitle("NovelMind Editor");
  } else {
    setWindowTitle(QString("NovelMind Editor - %1").arg(projectName));
  }
}

void NMMainWindow::saveLayout() {
  QSettings settings("NovelMind", "Editor");
  settings.setValue("mainwindow/geometry", saveGeometry());
  settings.setValue("mainwindow/state", saveState());
}

void NMMainWindow::restoreLayout() {
  QSettings settings("NovelMind", "Editor");
  restoreGeometry(settings.value("mainwindow/geometry").toByteArray());
  restoreState(settings.value("mainwindow/state").toByteArray());
}

void NMMainWindow::resetToDefaultLayout() {
  // Remove saved layout
  QSettings settings("NovelMind", "Editor");
  settings.remove("mainwindow/geometry");
  settings.remove("mainwindow/state");

  // Recreate default layout
  createDefaultLayout();
}

void NMMainWindow::closeEvent(QCloseEvent *event) {
  // TODO: Check for unsaved changes and prompt user

  saveLayout();
  event->accept();
}

} // namespace NovelMind::editor::qt
