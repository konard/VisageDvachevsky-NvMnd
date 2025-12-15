#pragma once

/**
 * @file nm_timeline_panel.hpp
 * @brief Timeline editor for keyframe-based animations and events
 *
 * Provides:
 * - Multiple tracks (audio, animation, events)
 * - Keyframe editing with handles
 * - Playback controls
 * - Frame-accurate scrubbing
 * - Track grouping and filtering
 */

#include "NovelMind/editor/qt/nm_dock_panel.hpp"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMap>
#include <QToolBar>
#include <QVector>
#include <QWidget>

class QToolBar;
class QPushButton;
class QSlider;
class QLabel;
class QSpinBox;
class QGraphicsItem;

namespace NovelMind::editor::qt {

/**
 * @brief Types of timeline tracks
 */
enum class TimelineTrackType {
  Audio,
  Animation,
  Event,
  Camera,
  Character,
  Effect
};

/**
 * @brief Keyframe data structure
 */
struct Keyframe {
  int frame;
  QVariant value;
  QString interpolation; // "linear", "ease", "step"
};

/**
 * @brief Timeline track containing keyframes
 */
class TimelineTrack {
public:
  QString name;
  TimelineTrackType type;
  bool visible = true;
  bool locked = false;
  QColor color;
  QVector<Keyframe> keyframes;

  void addKeyframe(int frame, const QVariant &value);
  void removeKeyframe(int frame);
  Keyframe *getKeyframe(int frame);
};

/**
 * @brief Timeline editor panel
 *
 * Professional timeline editor similar to those in Unity, Unreal, After
 * Effects. Supports multiple tracks, keyframe editing, playback, and frame
 * scrubbing.
 */
class NMTimelinePanel : public NMDockPanel {
  Q_OBJECT

public:
  /**
   * @brief Construct timeline panel
   * @param parent Parent widget
   */
  explicit NMTimelinePanel(QWidget *parent = nullptr);

  /**
   * @brief Destructor
   */
  ~NMTimelinePanel() override;

  /**
   * @brief Initialize the panel
   */
  void onInitialize() override;

  /**
   * @brief Shutdown the panel
   */
  void onShutdown() override;

  /**
   * @brief Update panel (called every frame)
   */
  void onUpdate(double deltaTime) override;

signals:
  /**
   * @brief Emitted when playback frame changes
   */
  void frameChanged(int frame);

  /**
   * @brief Emitted when a keyframe is added/modified
   */
  void keyframeModified(const QString &trackName, int frame);

  /**
   * @brief Emitted when playback state changes
   */
  void playbackStateChanged(bool playing);

public slots:
  /**
   * @brief Set the current frame
   */
  void setCurrentFrame(int frame);

  /**
   * @brief Play/pause timeline
   */
  void togglePlayback();

  /**
   * @brief Stop playback and return to start
   */
  void stopPlayback();

  /**
   * @brief Step forward one frame
   */
  void stepForward();

  /**
   * @brief Step backward one frame
   */
  void stepBackward();

  /**
   * @brief Add a new track
   */
  void addTrack(TimelineTrackType type, const QString &name);

  /**
   * @brief Remove a track
   */
  void removeTrack(const QString &name);

  /**
   * @brief Add keyframe at current frame
   */
  void addKeyframeAtCurrent(const QString &trackName, const QVariant &value);

  /**
   * @brief Zoom timeline view
   */
  void zoomIn();
  void zoomOut();
  void zoomToFit();

private:
  void setupUI();
  void setupToolbar();
  void setupPlaybackControls();
  void setupTrackView();

  void updatePlayhead();
  void updateFrameDisplay();
  void renderTracks();

  int frameToX(int frame) const;
  int xToFrame(int x) const;

  // UI Components
  QToolBar *m_toolbar = nullptr;
  QPushButton *m_btnPlay = nullptr;
  QPushButton *m_btnStop = nullptr;
  QPushButton *m_btnStepBack = nullptr;
  QPushButton *m_btnStepForward = nullptr;
  QSpinBox *m_frameSpinBox = nullptr;
  QLabel *m_timeLabel = nullptr;
  QPushButton *m_btnZoomIn = nullptr;
  QPushButton *m_btnZoomOut = nullptr;
  QPushButton *m_btnZoomFit = nullptr;

  // Timeline view
  QGraphicsView *m_timelineView = nullptr;
  QGraphicsScene *m_timelineScene = nullptr;
  QGraphicsLineItem *m_playheadItem = nullptr;

  // State
  QMap<QString, TimelineTrack *> m_tracks;
  int m_currentFrame = 0;
  int m_totalFrames = 300; // 10 seconds at 30fps
  int m_fps = 30;
  bool m_playing = false;
  double m_playbackTime = 0.0;
  float m_zoom = 1.0f;
  int m_pixelsPerFrame = 4;

  // Playback
  bool m_loop = true;
  int m_playbackStartFrame = 0;
  int m_playbackEndFrame = 300;

  static constexpr int TRACK_HEIGHT = 32;
  static constexpr int TRACK_HEADER_WIDTH = 150;
  static constexpr int TIMELINE_MARGIN = 20;
};

} // namespace NovelMind::editor::qt
