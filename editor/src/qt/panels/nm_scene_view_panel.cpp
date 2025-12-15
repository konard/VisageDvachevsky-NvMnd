#include "NovelMind/editor/qt/panels/nm_scene_view_panel.hpp"
#include "NovelMind/editor/qt/nm_icon_manager.hpp"
#include "NovelMind/editor/qt/nm_style_manager.hpp"

#include <QAbstractButton>
#include <QAction>
#include <QButtonGroup>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsSceneMouseEvent>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWheelEvent>

namespace NovelMind::editor::qt {

// ============================================================================
// NMSceneObject
// ============================================================================

NMSceneObject::NMSceneObject(const QString &id, NMSceneObjectType type,
                             QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent), m_id(id), m_name(id), m_objectType(type) {
  setFlag(ItemIsMovable, true);
  setFlag(ItemIsSelectable, true);
  setFlag(ItemSendsGeometryChanges, true);
  setAcceptHoverEvents(true);

  // Create placeholder pixmap based on type
  QPixmap pixmap(200, 300);
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing);

  const auto &palette = NMStyleManager::instance().palette();

  // Draw different shapes based on type
  switch (type) {
  case NMSceneObjectType::Background:
    painter.fillRect(pixmap.rect(), QColor(60, 60, 80, 180));
    painter.setPen(QPen(palette.textPrimary, 2));
    painter.drawText(pixmap.rect(), Qt::AlignCenter, "Background");
    break;

  case NMSceneObjectType::Character:
    painter.setBrush(QColor(100, 150, 200, 180));
    painter.setPen(QPen(palette.textPrimary, 2));
    painter.drawEllipse(50, 30, 100, 120); // Head
    painter.drawRect(70, 150, 60, 100);    // Body
    painter.drawText(pixmap.rect(), Qt::AlignCenter | Qt::AlignBottom,
                     "Character");
    break;

  case NMSceneObjectType::UI:
    painter.fillRect(0, 0, 200, 100, QColor(150, 150, 150, 180));
    painter.setPen(QPen(palette.textPrimary, 2));
    painter.drawText(0, 0, 200, 100, Qt::AlignCenter, "UI Element");
    break;

  case NMSceneObjectType::Effect:
    painter.setBrush(QColor(200, 100, 100, 180));
    painter.setPen(QPen(palette.textPrimary, 2));
    painter.drawEllipse(50, 50, 100, 100);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, "Effect");
    break;
  }

  setPixmap(pixmap);
}

void NMSceneObject::setSelected(bool selected) {
  m_selected = selected;
  QGraphicsPixmapItem::setSelected(selected);
  update();
}

void NMSceneObject::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *option,
                          QWidget *widget) {
  // Draw the pixmap
  QGraphicsPixmapItem::paint(painter, option, widget);

  // Draw selection outline
  if (m_selected || isSelected()) {
    const auto &palette = NMStyleManager::instance().palette();
    painter->setPen(QPen(palette.accentPrimary, 3, Qt::SolidLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boundingRect().adjusted(1, 1, -1, -1));

    // Draw corner handles
    QRectF bounds = boundingRect();
    int handleSize = 8;
    painter->setBrush(palette.accentPrimary);
    painter->drawRect(static_cast<int>(bounds.left()),
                      static_cast<int>(bounds.top()), handleSize, handleSize);
    painter->drawRect(static_cast<int>(bounds.right() - handleSize),
                      static_cast<int>(bounds.top()), handleSize, handleSize);
    painter->drawRect(static_cast<int>(bounds.left()),
                      static_cast<int>(bounds.bottom() - handleSize),
                      handleSize, handleSize);
    painter->drawRect(static_cast<int>(bounds.right() - handleSize),
                      static_cast<int>(bounds.bottom() - handleSize),
                      handleSize, handleSize);
  }
}

void NMSceneObject::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    event->accept();
  }
  QGraphicsPixmapItem::mousePressEvent(event);
}

void NMSceneObject::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
  QGraphicsPixmapItem::mouseMoveEvent(event);
}

void NMSceneObject::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
  QGraphicsPixmapItem::mouseReleaseEvent(event);
}

// ============================================================================
// NMTransformGizmo
// ============================================================================

NMTransformGizmo::NMTransformGizmo(QGraphicsItem *parent)
    : QGraphicsItemGroup(parent) {
  setFlag(ItemIgnoresTransformations, true);
  createMoveGizmo();
}

void NMTransformGizmo::setMode(GizmoMode mode) {
  if (m_mode == mode)
    return;

  m_mode = mode;
  clearGizmo();

  switch (mode) {
  case GizmoMode::Move:
    createMoveGizmo();
    break;
  case GizmoMode::Rotate:
    createRotateGizmo();
    break;
  case GizmoMode::Scale:
    createScaleGizmo();
    break;
  }

  updatePosition();
}

void NMTransformGizmo::setTargetObject(NMSceneObject *object) {
  m_targetObject = object;
  updatePosition();
  setVisible(object != nullptr);
}

void NMTransformGizmo::updatePosition() {
  if (m_targetObject) {
    QRectF bounds = m_targetObject->boundingRect();
    QPointF center = m_targetObject->pos() + bounds.center();
    setPos(center);
  }
}

void NMTransformGizmo::createMoveGizmo() {
  const auto &palette = NMStyleManager::instance().palette();
  qreal arrowLength = 60;
  qreal arrowHeadSize = 12;

  // X axis (Red)
  auto *xLine = new QGraphicsLineItem(0, 0, arrowLength, 0, this);
  xLine->setPen(QPen(QColor(220, 50, 50), 3));
  addToGroup(xLine);

  QPolygonF xArrow;
  xArrow << QPointF(arrowLength, 0)
         << QPointF(arrowLength - arrowHeadSize, -arrowHeadSize / 2)
         << QPointF(arrowLength - arrowHeadSize, arrowHeadSize / 2);
  auto *xArrowHead = new QGraphicsPolygonItem(xArrow, this);
  xArrowHead->setBrush(QColor(220, 50, 50));
  xArrowHead->setPen(Qt::NoPen);
  addToGroup(xArrowHead);

  // Y axis (Green)
  auto *yLine = new QGraphicsLineItem(0, 0, 0, arrowLength, this);
  yLine->setPen(QPen(QColor(50, 220, 50), 3));
  addToGroup(yLine);

  QPolygonF yArrow;
  yArrow << QPointF(0, arrowLength)
         << QPointF(-arrowHeadSize / 2, arrowLength - arrowHeadSize)
         << QPointF(arrowHeadSize / 2, arrowLength - arrowHeadSize);
  auto *yArrowHead = new QGraphicsPolygonItem(yArrow, this);
  yArrowHead->setBrush(QColor(50, 220, 50));
  yArrowHead->setPen(Qt::NoPen);
  addToGroup(yArrowHead);

  // Center circle
  auto *center = new QGraphicsEllipseItem(-6, -6, 12, 12, this);
  center->setBrush(palette.accentPrimary);
  center->setPen(QPen(palette.textPrimary, 2));
  addToGroup(center);
}

void NMTransformGizmo::createRotateGizmo() {
  const auto &palette = NMStyleManager::instance().palette();
  qreal radius = 60;

  // Outer circle
  auto *circle =
      new QGraphicsEllipseItem(-radius, -radius, radius * 2, radius * 2, this);
  circle->setPen(QPen(palette.accentPrimary, 3));
  circle->setBrush(Qt::NoBrush);
  addToGroup(circle);

  // Rotation handles at cardinal points
  for (int angle = 0; angle < 360; angle += 90) {
    qreal rad = qDegreesToRadians(static_cast<qreal>(angle));
    qreal x = radius * qCos(rad);
    qreal y = radius * qSin(rad);
    auto *handle = new QGraphicsEllipseItem(x - 6, y - 6, 12, 12, this);
    handle->setBrush(palette.accentPrimary);
    handle->setPen(Qt::NoPen);
    addToGroup(handle);
  }
}

void NMTransformGizmo::createScaleGizmo() {
  const auto &palette = NMStyleManager::instance().palette();
  qreal size = 50;

  // Bounding box
  auto *box = new QGraphicsRectItem(-size, -size, size * 2, size * 2, this);
  box->setPen(QPen(palette.accentPrimary, 2, Qt::DashLine));
  box->setBrush(Qt::NoBrush);
  addToGroup(box);

  // Corner handles
  QList<QPointF> corners = {QPointF(-size, -size), QPointF(size, -size),
                            QPointF(-size, size), QPointF(size, size)};

  for (const auto &corner : corners) {
    auto *handle =
        new QGraphicsRectItem(corner.x() - 6, corner.y() - 6, 12, 12, this);
    handle->setBrush(palette.accentPrimary);
    handle->setPen(Qt::NoPen);
    addToGroup(handle);
  }
}

void NMTransformGizmo::clearGizmo() {
  for (auto *item : childItems()) {
    removeFromGroup(item);
    delete item;
  }
}

// ============================================================================
// NMSceneGraphicsScene
// ============================================================================

NMSceneGraphicsScene::NMSceneGraphicsScene(QObject *parent)
    : QGraphicsScene(parent) {
  // Set a large scene rect for scrolling
  setSceneRect(-5000, -5000, 10000, 10000);

  // Create gizmo
  m_gizmo = new NMTransformGizmo();
  m_gizmo->setVisible(false);
  addItem(m_gizmo);
}

NMSceneGraphicsScene::~NMSceneGraphicsScene() {
  // Scene objects are owned by the scene and will be deleted automatically
}

void NMSceneGraphicsScene::setGridVisible(bool visible) {
  m_gridVisible = visible;
  invalidate(sceneRect(), BackgroundLayer);
}

void NMSceneGraphicsScene::setGridSize(qreal size) {
  m_gridSize = size;
  if (m_gridVisible) {
    invalidate(sceneRect(), BackgroundLayer);
  }
}

void NMSceneGraphicsScene::addSceneObject(NMSceneObject *object) {
  if (!object)
    return;

  m_sceneObjects.append(object);
  addItem(object);

  // Note: Position tracking should be implemented through
  // NMSceneObject::itemChange() or via a custom signal/slot mechanism in
  // NMSceneObject if it inherits from QObject
}

void NMSceneGraphicsScene::removeSceneObject(const QString &objectId) {
  for (int i = 0; i < m_sceneObjects.size(); ++i) {
    if (m_sceneObjects[i]->id() == objectId) {
      NMSceneObject *obj = m_sceneObjects.takeAt(i);
      if (obj == m_selectedObject) {
        m_selectedObject = nullptr;
        updateGizmo();
      }
      removeItem(obj);
      delete obj;
      break;
    }
  }
}

NMSceneObject *
NMSceneGraphicsScene::findSceneObject(const QString &objectId) const {
  for (auto *obj : m_sceneObjects) {
    if (obj->id() == objectId)
      return obj;
  }
  return nullptr;
}

void NMSceneGraphicsScene::selectObject(const QString &objectId) {
  // Clear previous selection
  if (m_selectedObject) {
    m_selectedObject->setSelected(false);
  }

  // Select new object
  m_selectedObject = findSceneObject(objectId);
  if (m_selectedObject) {
    m_selectedObject->setSelected(true);
    updateGizmo();
    emit objectSelected(objectId);
  }
}

void NMSceneGraphicsScene::clearSelection() {
  if (m_selectedObject) {
    m_selectedObject->setSelected(false);
    m_selectedObject = nullptr;
    updateGizmo();
    emit objectSelected(QString());
  }
}

void NMSceneGraphicsScene::setGizmoMode(NMTransformGizmo::GizmoMode mode) {
  m_gizmo->setMode(mode);
}

void NMSceneGraphicsScene::drawBackground(QPainter *painter,
                                          const QRectF &rect) {
  const auto &palette = NMStyleManager::instance().palette();

  // Fill background
  painter->fillRect(rect, palette.bgDarkest);

  if (!m_gridVisible)
    return;

  // Draw grid
  painter->setPen(QPen(palette.gridLine, 1));

  // Calculate grid bounds
  qreal left = rect.left() - std::fmod(rect.left(), m_gridSize);
  qreal top = rect.top() - std::fmod(rect.top(), m_gridSize);

  // Draw minor grid lines
  QVector<QLineF> lines;
  for (qreal x = left; x < rect.right(); x += m_gridSize) {
    lines.append(QLineF(x, rect.top(), x, rect.bottom()));
  }
  for (qreal y = top; y < rect.bottom(); y += m_gridSize) {
    lines.append(QLineF(rect.left(), y, rect.right(), y));
  }
  painter->drawLines(lines);

  // Draw major grid lines (every 8 minor lines)
  painter->setPen(QPen(palette.gridMajor, 1));
  qreal majorSize = m_gridSize * 8;
  left = rect.left() - std::fmod(rect.left(), majorSize);
  top = rect.top() - std::fmod(rect.top(), majorSize);

  lines.clear();
  for (qreal x = left; x < rect.right(); x += majorSize) {
    lines.append(QLineF(x, rect.top(), x, rect.bottom()));
  }
  for (qreal y = top; y < rect.bottom(); y += majorSize) {
    lines.append(QLineF(rect.left(), y, rect.right(), y));
  }
  painter->drawLines(lines);

  // Draw origin axes
  painter->setPen(QPen(palette.accentPrimary, 2));
  if (rect.left() <= 0 && rect.right() >= 0) {
    painter->drawLine(QLineF(0, rect.top(), 0, rect.bottom()));
  }
  if (rect.top() <= 0 && rect.bottom() >= 0) {
    painter->drawLine(QLineF(rect.left(), 0, rect.right(), 0));
  }
}

void NMSceneGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    // Check if we clicked on a scene object
    QGraphicsItem *item = itemAt(event->scenePos(), QTransform());

    // Skip gizmo items
    while (item && item->parentItem()) {
      if (item->parentItem() == m_gizmo) {
        // Clicked on gizmo, let default handling work
        QGraphicsScene::mousePressEvent(event);
        return;
      }
      item = item->parentItem();
    }

    // Check if it's a scene object
    auto *sceneObj = dynamic_cast<NMSceneObject *>(item);
    if (sceneObj) {
      selectObject(sceneObj->id());
    } else {
      // Clicked on empty space, clear selection
      clearSelection();
    }
  }

  QGraphicsScene::mousePressEvent(event);
}

void NMSceneGraphicsScene::updateGizmo() {
  m_gizmo->setTargetObject(m_selectedObject);
}

// ============================================================================
// NMSceneInfoOverlay
// ============================================================================

NMSceneInfoOverlay::NMSceneInfoOverlay(QWidget *parent) : QWidget(parent) {
  setAttribute(Qt::WA_TransparentForMouseEvents);
  setAttribute(Qt::WA_NoSystemBackground);

  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(10, 10, 10, 10);
  layout->setSpacing(5);

  const auto &palette = NMStyleManager::instance().palette();

  // Cursor position label
  m_cursorLabel = new QLabel(this);
  m_cursorLabel->setStyleSheet(
      QString("QLabel {"
              "  background-color: rgba(45, 45, 48, 200);"
              "  color: %1;"
              "  padding: 5px 10px;"
              "  border-radius: 3px;"
              "  font-family: 'Consolas', 'Monaco', monospace;"
              "  font-size: 11px;"
              "}")
          .arg(palette.textPrimary.name()));
  layout->addWidget(m_cursorLabel);

  // Selected object label
  m_objectLabel = new QLabel(this);
  m_objectLabel->setStyleSheet(
      QString("QLabel {"
              "  background-color: rgba(0, 120, 212, 200);"
              "  color: %1;"
              "  padding: 5px 10px;"
              "  border-radius: 3px;"
              "  font-family: 'Consolas', 'Monaco', monospace;"
              "  font-size: 11px;"
              "}")
          .arg(palette.textPrimary.name()));
  m_objectLabel->setVisible(false);
  layout->addWidget(m_objectLabel);

  layout->addStretch();

  updateDisplay();
}

void NMSceneInfoOverlay::setCursorPosition(const QPointF &pos) {
  m_cursorPos = pos;
  updateDisplay();
}

void NMSceneInfoOverlay::setSelectedObjectInfo(const QString &name,
                                               const QPointF &pos) {
  m_objectName = name;
  m_objectPos = pos;
  m_hasSelection = true;
  updateDisplay();
}

void NMSceneInfoOverlay::clearSelectedObjectInfo() {
  m_hasSelection = false;
  updateDisplay();
}

void NMSceneInfoOverlay::updateDisplay() {
  m_cursorLabel->setText(QString("Cursor: X: %1  Y: %2")
                             .arg(m_cursorPos.x(), 7, 'f', 1)
                             .arg(m_cursorPos.y(), 7, 'f', 1));

  if (m_hasSelection) {
    m_objectLabel->setText(QString("%1 - X: %2  Y: %3")
                               .arg(m_objectName)
                               .arg(m_objectPos.x(), 7, 'f', 1)
                               .arg(m_objectPos.y(), 7, 'f', 1));
    m_objectLabel->setVisible(true);
  } else {
    m_objectLabel->setVisible(false);
  }
}

// ============================================================================
// NMSceneGraphicsView
// ============================================================================

NMSceneGraphicsView::NMSceneGraphicsView(QWidget *parent)
    : QGraphicsView(parent) {
  setRenderHint(QPainter::Antialiasing);
  setRenderHint(QPainter::SmoothPixmapTransform);
  setViewportUpdateMode(FullViewportUpdate);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setTransformationAnchor(AnchorUnderMouse);
  setResizeAnchor(AnchorViewCenter);
  setDragMode(NoDrag);

  // Set background
  setBackgroundBrush(Qt::NoBrush);

  // Enable mouse tracking for cursor position
  setMouseTracking(true);
}

void NMSceneGraphicsView::setZoomLevel(qreal zoom) {
  zoom = qBound(0.1, zoom, 10.0);
  if (qFuzzyCompare(m_zoomLevel, zoom))
    return;

  qreal scaleFactor = zoom / m_zoomLevel;
  m_zoomLevel = zoom;

  scale(scaleFactor, scaleFactor);
  emit zoomChanged(m_zoomLevel);
}

void NMSceneGraphicsView::centerOnScene() { centerOn(0, 0); }

void NMSceneGraphicsView::fitToScene() {
  if (scene() && !scene()->items().isEmpty()) {
    fitInView(scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
    m_zoomLevel = transform().m11();
    emit zoomChanged(m_zoomLevel);
  }
}

void NMSceneGraphicsView::wheelEvent(QWheelEvent *event) {
  // Zoom with mouse wheel
  qreal factor = 1.15;
  if (event->angleDelta().y() < 0) {
    factor = 1.0 / factor;
  }

  setZoomLevel(m_zoomLevel * factor);
  event->accept();
}

void NMSceneGraphicsView::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::MiddleButton) {
    // Start panning
    m_isPanning = true;
    m_lastPanPoint = event->pos();
    setCursor(Qt::ClosedHandCursor);
    event->accept();
    return;
  }

  QGraphicsView::mousePressEvent(event);
}

void NMSceneGraphicsView::mouseMoveEvent(QMouseEvent *event) {
  // Emit cursor position in scene coordinates
  QPointF scenePos = mapToScene(event->pos());
  emit cursorPositionChanged(scenePos);

  if (m_isPanning) {
    QPoint delta = event->pos() - m_lastPanPoint;
    m_lastPanPoint = event->pos();

    horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
    verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
    event->accept();
    return;
  }

  QGraphicsView::mouseMoveEvent(event);
}

void NMSceneGraphicsView::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::MiddleButton && m_isPanning) {
    m_isPanning = false;
    setCursor(Qt::ArrowCursor);
    event->accept();
    return;
  }

  QGraphicsView::mouseReleaseEvent(event);
}

// ============================================================================
// NMSceneViewPanel
// ============================================================================

NMSceneViewPanel::NMSceneViewPanel(QWidget *parent)
    : NMDockPanel(tr("Scene View"), parent) {
  setPanelId("SceneView");
  setupContent();
  setupToolBar();
}

NMSceneViewPanel::~NMSceneViewPanel() = default;

void NMSceneViewPanel::onInitialize() {
  // Center the view on origin
  if (m_view) {
    m_view->centerOnScene();
  }

  // Add demo objects
  addDemoObjects();
}

void NMSceneViewPanel::onUpdate(double /*deltaTime*/) {
  // Continuous updates for animations, etc.
  // For now, no continuous update needed
}

void NMSceneViewPanel::setGridVisible(bool visible) {
  if (m_scene) {
    m_scene->setGridVisible(visible);
  }
}

void NMSceneViewPanel::setZoomLevel(qreal zoom) {
  if (m_view) {
    m_view->setZoomLevel(zoom);
  }
}

void NMSceneViewPanel::setGizmoMode(NMTransformGizmo::GizmoMode mode) {
  if (m_scene) {
    m_scene->setGizmoMode(mode);
  }
}

void NMSceneViewPanel::addDemoObjects() {
  if (!m_scene)
    return;

  // Add background
  auto *background =
      new NMSceneObject("bg_main", NMSceneObjectType::Background);
  background->setName("Main Background");
  background->setPos(-100, -150);
  m_scene->addSceneObject(background);

  // Add character 1
  auto *character1 =
      new NMSceneObject("char_protagonist", NMSceneObjectType::Character);
  character1->setName("Protagonist");
  character1->setPos(-250, -100);
  m_scene->addSceneObject(character1);

  // Add character 2
  auto *character2 =
      new NMSceneObject("char_companion", NMSceneObjectType::Character);
  character2->setName("Companion");
  character2->setPos(150, -100);
  m_scene->addSceneObject(character2);

  // Add UI element
  auto *uiElement = new NMSceneObject("ui_dialogue_box", NMSceneObjectType::UI);
  uiElement->setName("Dialogue Box");
  uiElement->setPos(-100, 250);
  m_scene->addSceneObject(uiElement);
}

void NMSceneViewPanel::setupToolBar() {
  m_toolBar = new QToolBar(this);
  m_toolBar->setObjectName("SceneViewToolBar");
  m_toolBar->setIconSize(QSize(16, 16));

  auto &iconMgr = NMIconManager::instance();

  // Zoom controls
  QAction *actionZoomIn =
      m_toolBar->addAction(iconMgr.getIcon("zoom-in"), tr("Zoom In"));
  actionZoomIn->setToolTip(tr("Zoom In (Scroll Up)"));
  connect(actionZoomIn, &QAction::triggered, this, &NMSceneViewPanel::onZoomIn);

  QAction *actionZoomOut =
      m_toolBar->addAction(iconMgr.getIcon("zoom-out"), tr("Zoom Out"));
  actionZoomOut->setToolTip(tr("Zoom Out (Scroll Down)"));
  connect(actionZoomOut, &QAction::triggered, this,
          &NMSceneViewPanel::onZoomOut);

  QAction *actionZoomReset =
      m_toolBar->addAction(iconMgr.getIcon("zoom-fit"), tr("Reset"));
  actionZoomReset->setToolTip(tr("Reset Zoom (1:1)"));
  connect(actionZoomReset, &QAction::triggered, this,
          &NMSceneViewPanel::onZoomReset);

  m_toolBar->addSeparator();

  // Grid toggle
  QAction *actionToggleGrid = m_toolBar->addAction(tr("Grid"));
  actionToggleGrid->setToolTip(tr("Toggle Grid (G)"));
  actionToggleGrid->setCheckable(true);
  actionToggleGrid->setChecked(true);
  connect(actionToggleGrid, &QAction::toggled, this,
          &NMSceneViewPanel::onToggleGrid);

  m_toolBar->addSeparator();

  // Gizmo mode buttons (exclusive)
  auto *gizmoGroup = new QButtonGroup(this);

  QAction *actionMove = m_toolBar->addAction(tr("Move"));
  actionMove->setToolTip(tr("Move Gizmo (W)"));
  actionMove->setCheckable(true);
  actionMove->setChecked(true);
  gizmoGroup->addButton(m_toolBar->widgetForAction(actionMove)
                            ? qobject_cast<QAbstractButton *>(
                                  m_toolBar->widgetForAction(actionMove))
                            : nullptr);
  connect(actionMove, &QAction::triggered, this,
          &NMSceneViewPanel::onGizmoModeMove);

  QAction *actionRotate = m_toolBar->addAction(tr("Rotate"));
  actionRotate->setToolTip(tr("Rotate Gizmo (E)"));
  actionRotate->setCheckable(true);
  connect(actionRotate, &QAction::triggered, this,
          &NMSceneViewPanel::onGizmoModeRotate);

  QAction *actionScale = m_toolBar->addAction(tr("Scale"));
  actionScale->setToolTip(tr("Scale Gizmo (R)"));
  actionScale->setCheckable(true);
  connect(actionScale, &QAction::triggered, this,
          &NMSceneViewPanel::onGizmoModeScale);

  // Insert toolbar at top of content widget
  if (auto *layout = qobject_cast<QVBoxLayout *>(m_contentWidget->layout())) {
    layout->insertWidget(0, m_toolBar);
  }
}

void NMSceneViewPanel::setupContent() {
  m_contentWidget = new QWidget(this);
  auto *layout = new QVBoxLayout(m_contentWidget);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  // Create graphics scene and view
  m_scene = new NMSceneGraphicsScene(this);
  m_view = new NMSceneGraphicsView(m_contentWidget);
  m_view->setScene(m_scene);

  // Create info overlay
  m_infoOverlay = new NMSceneInfoOverlay(m_view);
  m_infoOverlay->setGeometry(m_view->rect());

  layout->addWidget(m_view);

  setContentWidget(m_contentWidget);

  // Connect signals
  connect(m_view, &NMSceneGraphicsView::cursorPositionChanged, this,
          &NMSceneViewPanel::onCursorPositionChanged);
  connect(m_scene, &NMSceneGraphicsScene::objectSelected, this,
          &NMSceneViewPanel::onSceneObjectSelected);
  connect(m_scene, &NMSceneGraphicsScene::objectPositionChanged, this,
          &NMSceneViewPanel::onObjectPositionChanged);
}

void NMSceneViewPanel::updateInfoOverlay() {
  // Resize overlay to match view
  if (m_infoOverlay && m_view) {
    m_infoOverlay->setGeometry(m_view->rect());
  }
}

void NMSceneViewPanel::onZoomIn() {
  if (m_view) {
    m_view->setZoomLevel(m_view->zoomLevel() * 1.25);
  }
}

void NMSceneViewPanel::onZoomOut() {
  if (m_view) {
    m_view->setZoomLevel(m_view->zoomLevel() / 1.25);
  }
}

void NMSceneViewPanel::onZoomReset() {
  if (m_view) {
    m_view->setZoomLevel(1.0);
    m_view->centerOnScene();
  }
}

void NMSceneViewPanel::onToggleGrid() {
  if (m_scene) {
    m_scene->setGridVisible(!m_scene->isGridVisible());
  }
}

void NMSceneViewPanel::onGizmoModeMove() {
  setGizmoMode(NMTransformGizmo::GizmoMode::Move);
}

void NMSceneViewPanel::onGizmoModeRotate() {
  setGizmoMode(NMTransformGizmo::GizmoMode::Rotate);
}

void NMSceneViewPanel::onGizmoModeScale() {
  setGizmoMode(NMTransformGizmo::GizmoMode::Scale);
}

void NMSceneViewPanel::onCursorPositionChanged(const QPointF &scenePos) {
  if (m_infoOverlay) {
    m_infoOverlay->setCursorPosition(scenePos);
  }
}

void NMSceneViewPanel::onSceneObjectSelected(const QString &objectId) {
  if (m_infoOverlay) {
    if (objectId.isEmpty()) {
      m_infoOverlay->clearSelectedObjectInfo();
    } else if (auto *obj = m_scene->findSceneObject(objectId)) {
      m_infoOverlay->setSelectedObjectInfo(obj->name(), obj->pos());
    }
  }

  // Forward to main window's selection system
  emit objectSelected(objectId);
}

void NMSceneViewPanel::onObjectPositionChanged(const QString &objectId,
                                               const QPointF &position) {
  if (m_infoOverlay && m_scene) {
    if (auto *obj = m_scene->findSceneObject(objectId)) {
      m_infoOverlay->setSelectedObjectInfo(obj->name(), position);
    }
  }
}

} // namespace NovelMind::editor::qt
