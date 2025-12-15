#ifndef NOVELMIND_EDITOR_NM_PLAY_MODE_CONTROLLER_HPP
#define NOVELMIND_EDITOR_NM_PLAY_MODE_CONTROLLER_HPP

#include <QObject>
#include <QSet>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QVariantMap>

namespace NovelMind::editor::qt {

/**
 * @brief Central controller for Play-In-Editor mode
 *
 * Manages playback state, breakpoints, and runtime integration.
 * Singleton pattern for global access from all panels.
 */
class NMPlayModeController : public QObject {
  Q_OBJECT

public:
  enum PlayMode {
    Stopped, ///< Not running
    Playing, ///< Actively executing
    Paused   ///< Paused at breakpoint or user request
  };
  Q_ENUM(PlayMode)

  /// Get singleton instance
  static NMPlayModeController &instance();

  /// Delete copy/move constructors
  NMPlayModeController(const NMPlayModeController &) = delete;
  NMPlayModeController &operator=(const NMPlayModeController &) = delete;

  // === Playback Control ===

  /// Start or resume playback
  void play();

  /// Pause playback (can be resumed)
  void pause();

  /// Stop playback and reset state
  void stop();

  /// Execute one instruction (only works when paused)
  void stepForward();

  // === State Queries ===

  /// Get current play mode
  PlayMode playMode() const { return m_playMode; }

  /// Check if currently playing
  bool isPlaying() const { return m_playMode == Playing; }

  /// Check if currently paused
  bool isPaused() const { return m_playMode == Paused; }

  /// Check if stopped
  bool isStopped() const { return m_playMode == Stopped; }

  /// Get current executing node ID
  QString currentNodeId() const { return m_currentNodeId; }

  // === Breakpoint Management ===

  /// Toggle breakpoint on/off for a node
  void toggleBreakpoint(const QString &nodeId);

  /// Set breakpoint state explicitly
  void setBreakpoint(const QString &nodeId, bool enabled);

  /// Check if node has a breakpoint
  bool hasBreakpoint(const QString &nodeId) const;

  /// Get all breakpoint node IDs
  QSet<QString> breakpoints() const { return m_breakpoints; }

  /// Clear all breakpoints
  void clearAllBreakpoints();

  // === Variable Inspection ===

  /// Get all current runtime variables
  QVariantMap currentVariables() const { return m_variables; }

  /// Set a variable value (only works when paused)
  void setVariable(const QString &name, const QVariant &value);

  /// Get call stack (list of function names)
  QStringList callStack() const { return m_callStack; }

signals:
  // === Playback Signals ===

  /// Emitted when play mode changes
  void playModeChanged(PlayMode mode);

  /// Emitted when a breakpoint is hit
  void breakpointHit(const QString &nodeId);

  /// Emitted when execution reaches a new node
  void currentNodeChanged(const QString &nodeId);

  // === Data Signals ===

  /// Emitted when variables are updated
  void variablesChanged(const QVariantMap &variables);

  /// Emitted when call stack changes
  void callStackChanged(const QStringList &stack);

  // === Breakpoint Signals ===

  /// Emitted when breakpoints are modified
  void breakpointsChanged();

public slots:
  /// Load breakpoints from project settings
  void loadBreakpoints(const QString &projectPath);

  /// Save breakpoints to project settings
  void saveBreakpoints(const QString &projectPath);

private:
  explicit NMPlayModeController(QObject *parent = nullptr);
  ~NMPlayModeController() override = default;

  /// Start mock runtime simulation
  void startMockRuntime();

  /// Stop mock runtime simulation
  void stopMockRuntime();

  /// Simulate one execution step
  void simulateStep();

  /// Check if current node has breakpoint and pause if needed
  void checkBreakpoint();

  // === State ===
  PlayMode m_playMode = Stopped;
  QString m_currentNodeId;
  QSet<QString> m_breakpoints;
  QVariantMap m_variables;
  QStringList m_callStack;

  // === Mock Runtime (Phase 5.0) ===
  QTimer *m_mockTimer = nullptr;
  QStringList m_mockNodeSequence; // Demo nodes to execute
  int m_mockStepIndex = 0;

  // === Future: Real Runtime (Phase 5.1+) ===
  // QThread* m_runtimeThread = nullptr;
  // RuntimeWorker* m_worker = nullptr;
};

} // namespace NovelMind::editor::qt

#endif // NOVELMIND_EDITOR_NM_PLAY_MODE_CONTROLLER_HPP
