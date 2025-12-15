#pragma once

/**
 * @file nm_main_window.hpp
 * @brief Main window for the NovelMind Editor
 *
 * The central main window that contains:
 * - Menu bar with all editor actions
 * - Toolbar with common actions
 * - Docking framework for all panels
 * - Status bar with editor state information
 */

#include <QMainWindow>
#include <QTimer>
#include <QToolBar>
#include <memory>

class QMenuBar;
class QToolBar;
class QStatusBar;
class QAction;
class QActionGroup;
class QLabel;

namespace NovelMind::editor::qt {

// Forward declarations
class NMDockPanel;
class NMSceneViewPanel;
class NMStoryGraphPanel;
class NMInspectorPanel;
class NMConsolePanel;
class NMAssetBrowserPanel;
class NMHierarchyPanel;
class NMPlayToolbarPanel;
class NMDebugOverlayPanel;

/**
 * @brief Main application window for the NovelMind Editor
 */
class NMMainWindow : public QMainWindow {
  Q_OBJECT

public:
  /**
   * @brief Construct the main window
   * @param parent Parent widget (usually nullptr for main window)
   */
  explicit NMMainWindow(QWidget *parent = nullptr);

  /**
   * @brief Destructor
   */
  ~NMMainWindow() override;

  /**
   * @brief Initialize the main window and all panels
   * @return true if initialization succeeded
   */
  bool initialize();

  /**
   * @brief Shutdown and cleanup
   */
  void shutdown();

  // =========================================================================
  // Panel Access
  // =========================================================================

  [[nodiscard]] NMSceneViewPanel *sceneViewPanel() const {
    return m_sceneViewPanel;
  }
  [[nodiscard]] NMStoryGraphPanel *storyGraphPanel() const {
    return m_storyGraphPanel;
  }
  [[nodiscard]] NMInspectorPanel *inspectorPanel() const {
    return m_inspectorPanel;
  }
  [[nodiscard]] NMConsolePanel *consolePanel() const { return m_consolePanel; }
  [[nodiscard]] NMAssetBrowserPanel *assetBrowserPanel() const {
    return m_assetBrowserPanel;
  }
  [[nodiscard]] NMHierarchyPanel *hierarchyPanel() const {
    return m_hierarchyPanel;
  }
  [[nodiscard]] NMPlayToolbarPanel *playToolbarPanel() const {
    return m_playToolbarPanel;
  }
  [[nodiscard]] NMDebugOverlayPanel *debugOverlayPanel() const {
    return m_debugOverlayPanel;
  }

  // =========================================================================
  // Layout Management
  // =========================================================================

  /**
   * @brief Save the current window layout to settings
   */
  void saveLayout();

  /**
   * @brief Restore the window layout from settings
   */
  void restoreLayout();

  /**
   * @brief Reset to the default layout
   */
  void resetToDefaultLayout();

signals:
  /**
   * @brief Emitted when a new project should be created
   */
  void newProjectRequested();

  /**
   * @brief Emitted when a project should be opened
   */
  void openProjectRequested();

  /**
   * @brief Emitted when the current project should be saved
   */
  void saveProjectRequested();

  /**
   * @brief Emitted when undo is requested
   */
  void undoRequested();

  /**
   * @brief Emitted when redo is requested
   */
  void redoRequested();

  /**
   * @brief Emitted when play mode should start
   */
  void playRequested();

  /**
   * @brief Emitted when play mode should stop
   */
  void stopRequested();

public slots:
  /**
   * @brief Update all panels (called by timer)
   */
  void onUpdateTick();

  /**
   * @brief Show the about dialog
   */
  void showAboutDialog();

  /**
   * @brief Toggle panel visibility
   */
  void togglePanel(NMDockPanel *panel);

  /**
   * @brief Set status bar message
   */
  void setStatusMessage(const QString &message, int timeout = 0);

  /**
   * @brief Update the window title with project name
   */
  void updateWindowTitle(const QString &projectName = QString());

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  void setupMenuBar();
  void setupToolBar();
  void setupStatusBar();
  void setupPanels();
  void setupConnections();
  void setupShortcuts();
  void createDefaultLayout();

  // =========================================================================
  // Menu Actions
  // =========================================================================

  // File menu
  QAction *m_actionNewProject = nullptr;
  QAction *m_actionOpenProject = nullptr;
  QAction *m_actionSaveProject = nullptr;
  QAction *m_actionSaveProjectAs = nullptr;
  QAction *m_actionCloseProject = nullptr;
  QAction *m_actionExit = nullptr;

  // Edit menu
  QAction *m_actionUndo = nullptr;
  QAction *m_actionRedo = nullptr;
  QAction *m_actionCut = nullptr;
  QAction *m_actionCopy = nullptr;
  QAction *m_actionPaste = nullptr;
  QAction *m_actionDelete = nullptr;
  QAction *m_actionSelectAll = nullptr;
  QAction *m_actionPreferences = nullptr;

  // View menu
  QAction *m_actionResetLayout = nullptr;
  QAction *m_actionToggleSceneView = nullptr;
  QAction *m_actionToggleStoryGraph = nullptr;
  QAction *m_actionToggleInspector = nullptr;
  QAction *m_actionToggleConsole = nullptr;
  QAction *m_actionToggleAssetBrowser = nullptr;
  QAction *m_actionToggleHierarchy = nullptr;

  // Play menu
  QAction *m_actionPlay = nullptr;
  QAction *m_actionPause = nullptr;
  QAction *m_actionStop = nullptr;
  QAction *m_actionStepFrame = nullptr;

  // Help menu
  QAction *m_actionAbout = nullptr;
  QAction *m_actionDocumentation = nullptr;

  // =========================================================================
  // UI Components
  // =========================================================================

  QToolBar *m_mainToolBar = nullptr;
  QLabel *m_statusLabel = nullptr;

  // =========================================================================
  // Panels
  // =========================================================================

  NMSceneViewPanel *m_sceneViewPanel = nullptr;
  NMStoryGraphPanel *m_storyGraphPanel = nullptr;
  NMInspectorPanel *m_inspectorPanel = nullptr;
  NMConsolePanel *m_consolePanel = nullptr;
  NMAssetBrowserPanel *m_assetBrowserPanel = nullptr;
  NMHierarchyPanel *m_hierarchyPanel = nullptr;
  NMPlayToolbarPanel *m_playToolbarPanel = nullptr;
  NMDebugOverlayPanel *m_debugOverlayPanel = nullptr;

  // =========================================================================
  // State
  // =========================================================================

  QTimer *m_updateTimer = nullptr;
  bool m_initialized = false;
  static constexpr int UPDATE_INTERVAL_MS = 16; // ~60 FPS
};

} // namespace NovelMind::editor::qt
