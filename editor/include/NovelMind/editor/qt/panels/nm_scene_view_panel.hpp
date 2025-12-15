#pragma once

/**
 * @file nm_scene_view_panel.hpp
 * @brief Scene View panel for visual scene editing
 *
 * Displays the visual novel scene with:
 * - Background image
 * - Character sprites
 * - UI elements
 * - Selection highlighting
 * - Viewport controls (pan, zoom)
 */

#include "NovelMind/editor/qt/nm_dock_panel.hpp"
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>
#include <QToolBar>

namespace NovelMind::editor::qt {

// Forward declarations
class NMSceneObject;
class NMTransformGizmo;

/**
 * @brief Scene object types
 */
enum class NMSceneObjectType { Background, Character, UI, Effect };

/**
 * @brief Scene object representation
 */
class NMSceneObject : public QGraphicsPixmapItem {
public:
  explicit NMSceneObject(const QString &id, NMSceneObjectType type,
                         QGraphicsItem *parent = nullptr);

  [[nodiscard]] QString id() const { return m_id; }
  [[nodiscard]] NMSceneObjectType objectType() const { return m_objectType; }
  [[nodiscard]] QString name() const { return m_name; }

  void setName(const QString &name) { m_name = name; }
  void setSelected(bool selected);
  [[nodiscard]] bool isObjectSelected() const { return m_selected; }

protected:
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
  QString m_id;
  QString m_name;
  NMSceneObjectType m_objectType;
  bool m_selected = false;
};

/**
 * @brief Transform gizmo for moving objects
 */
class NMTransformGizmo : public QGraphicsItemGroup {
public:
  enum class GizmoMode { Move, Rotate, Scale };

  explicit NMTransformGizmo(QGraphicsItem *parent = nullptr);

  void setMode(GizmoMode mode);
  [[nodiscard]] GizmoMode mode() const { return m_mode; }

  void setTargetObject(NMSceneObject *object);
  [[nodiscard]] NMSceneObject *targetObject() const { return m_targetObject; }

  void updatePosition();

private:
  void createMoveGizmo();
  void createRotateGizmo();
  void createScaleGizmo();
  void clearGizmo();

  GizmoMode m_mode = GizmoMode::Move;
  NMSceneObject *m_targetObject = nullptr;
};

/**
 * @brief Graphics scene for the scene view
 */
class NMSceneGraphicsScene : public QGraphicsScene {
  Q_OBJECT

public:
  explicit NMSceneGraphicsScene(QObject *parent = nullptr);
  ~NMSceneGraphicsScene() override;

  void setGridVisible(bool visible);
  [[nodiscard]] bool isGridVisible() const { return m_gridVisible; }

  void setGridSize(qreal size);
  [[nodiscard]] qreal gridSize() const { return m_gridSize; }

  void addSceneObject(NMSceneObject *object);
  void removeSceneObject(const QString &objectId);
  [[nodiscard]] NMSceneObject *findSceneObject(const QString &objectId) const;
  [[nodiscard]] QList<NMSceneObject *> sceneObjects() const {
    return m_sceneObjects;
  }

  void selectObject(const QString &objectId);
  void clearSelection();
  [[nodiscard]] NMSceneObject *selectedObject() const {
    return m_selectedObject;
  }

  void setGizmoMode(NMTransformGizmo::GizmoMode mode);

signals:
  void objectSelected(const QString &objectId);
  void objectPositionChanged(const QString &objectId, const QPointF &position);

protected:
  void drawBackground(QPainter *painter, const QRectF &rect) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

private:
  void updateGizmo();

  bool m_gridVisible = true;
  qreal m_gridSize = 32.0;
  QList<NMSceneObject *> m_sceneObjects;
  NMSceneObject *m_selectedObject = nullptr;
  NMTransformGizmo *m_gizmo = nullptr;
};

/**
 * @brief Info overlay widget showing cursor and object info
 */
class NMSceneInfoOverlay : public QWidget {
  Q_OBJECT

public:
  explicit NMSceneInfoOverlay(QWidget *parent = nullptr);

  void setCursorPosition(const QPointF &pos);
  void setSelectedObjectInfo(const QString &name, const QPointF &pos);
  void clearSelectedObjectInfo();

private:
  void updateDisplay();

  QLabel *m_cursorLabel = nullptr;
  QLabel *m_objectLabel = nullptr;
  QPointF m_cursorPos;
  QString m_objectName;
  QPointF m_objectPos;
  bool m_hasSelection = false;
};

/**
 * @brief Graphics view with pan and zoom support
 */
class NMSceneGraphicsView : public QGraphicsView {
  Q_OBJECT

public:
  explicit NMSceneGraphicsView(QWidget *parent = nullptr);

  void setZoomLevel(qreal zoom);
  [[nodiscard]] qreal zoomLevel() const { return m_zoomLevel; }

  void centerOnScene();
  void fitToScene();

signals:
  void zoomChanged(qreal newZoom);
  void cursorPositionChanged(const QPointF &scenePos);

protected:
  void wheelEvent(QWheelEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  qreal m_zoomLevel = 1.0;
  bool m_isPanning = false;
  QPoint m_lastPanPoint;
};

/**
 * @brief Scene View panel for visual scene editing
 */
class NMSceneViewPanel : public NMDockPanel {
  Q_OBJECT

public:
  explicit NMSceneViewPanel(QWidget *parent = nullptr);
  ~NMSceneViewPanel() override;

  void onInitialize() override;
  void onUpdate(double deltaTime) override;

  /**
   * @brief Get the graphics scene
   */
  [[nodiscard]] NMSceneGraphicsScene *graphicsScene() const { return m_scene; }

  /**
   * @brief Get the graphics view
   */
  [[nodiscard]] NMSceneGraphicsView *graphicsView() const { return m_view; }

  /**
   * @brief Set grid visibility
   */
  void setGridVisible(bool visible);

  /**
   * @brief Set zoom level
   */
  void setZoomLevel(qreal zoom);

  /**
   * @brief Set gizmo mode
   */
  void setGizmoMode(NMTransformGizmo::GizmoMode mode);

  /**
   * @brief Add demo scene objects for testing
   */
  void addDemoObjects();

signals:
  void objectSelected(const QString &objectId);
  void objectDoubleClicked(const QString &objectId);

private slots:
  void onZoomIn();
  void onZoomOut();
  void onZoomReset();
  void onToggleGrid();
  void onGizmoModeMove();
  void onGizmoModeRotate();
  void onGizmoModeScale();
  void onCursorPositionChanged(const QPointF &scenePos);
  void onSceneObjectSelected(const QString &objectId);
  void onObjectPositionChanged(const QString &objectId,
                               const QPointF &position);

private:
  void setupToolBar();
  void setupContent();
  void updateInfoOverlay();

  NMSceneGraphicsScene *m_scene = nullptr;
  NMSceneGraphicsView *m_view = nullptr;
  QWidget *m_contentWidget = nullptr;
  QToolBar *m_toolBar = nullptr;
  NMSceneInfoOverlay *m_infoOverlay = nullptr;
};

} // namespace NovelMind::editor::qt
