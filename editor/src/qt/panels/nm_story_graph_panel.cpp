#include "NovelMind/editor/qt/panels/nm_story_graph_panel.hpp"
#include "NovelMind/editor/qt/nm_icon_manager.hpp"
#include "NovelMind/editor/qt/nm_play_mode_controller.hpp"
#include "NovelMind/editor/qt/nm_style_manager.hpp"
#include "NovelMind/editor/qt/nm_undo_manager.hpp"

#include <QAction>
#include <QFrame>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QScrollBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWheelEvent>

namespace NovelMind::editor::qt {

// ============================================================================
// NMGraphNodeItem
// ============================================================================

NMGraphNodeItem::NMGraphNodeItem(const QString &title, const QString &nodeType)
    : m_title(title), m_nodeType(nodeType) {
  setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
}

void NMGraphNodeItem::setTitle(const QString &title) {
  m_title = title;
  update();
}

void NMGraphNodeItem::setNodeType(const QString &type) {
  m_nodeType = type;
  update();
}

void NMGraphNodeItem::setSelected(bool selected) {
  m_isSelected = selected;
  QGraphicsItem::setSelected(selected);
  update();
}

void NMGraphNodeItem::setBreakpoint(bool hasBreakpoint) {
  m_hasBreakpoint = hasBreakpoint;
  update();
}

void NMGraphNodeItem::setCurrentlyExecuting(bool isExecuting) {
  m_isCurrentlyExecuting = isExecuting;
  update();
}

QRectF NMGraphNodeItem::boundingRect() const {
  return QRectF(0, 0, NODE_WIDTH, NODE_HEIGHT);
}

void NMGraphNodeItem::paint(QPainter *painter,
                            const QStyleOptionGraphicsItem * /*option*/,
                            QWidget * /*widget*/) {
  const auto &palette = NMStyleManager::instance().palette();

  painter->setRenderHint(QPainter::Antialiasing);

  // Node background
  QColor bgColor = m_isSelected ? palette.nodeSelected : palette.nodeDefault;
  painter->setBrush(bgColor);
  painter->setPen(QPen(palette.borderLight, 1));
  painter->drawRoundedRect(boundingRect(), CORNER_RADIUS, CORNER_RADIUS);

  // Header bar
  QRectF headerRect(0, 0, NODE_WIDTH, 24);
  painter->setBrush(palette.bgDark);
  painter->setPen(Qt::NoPen);
  QPainterPath headerPath;
  headerPath.addRoundedRect(headerRect, CORNER_RADIUS, CORNER_RADIUS);
  // Clip to top corners only
  QPainterPath clipPath;
  clipPath.addRect(QRectF(0, CORNER_RADIUS, NODE_WIDTH, 24 - CORNER_RADIUS));
  headerPath = headerPath.united(clipPath);
  painter->drawPath(headerPath);

  // Node type (header)
  painter->setPen(palette.textSecondary);
  painter->setFont(NMStyleManager::instance().defaultFont());
  painter->drawText(headerRect.adjusted(8, 0, -8, 0),
                    Qt::AlignVCenter | Qt::AlignLeft, m_nodeType);

  // Node title (body)
  QRectF titleRect(8, 30, NODE_WIDTH - 16, NODE_HEIGHT - 38);
  painter->setPen(palette.textPrimary);
  QFont boldFont = NMStyleManager::instance().defaultFont();
  boldFont.setBold(true);
  painter->setFont(boldFont);
  painter->drawText(titleRect, Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap,
                    m_title);

  // Selection highlight
  if (m_isSelected) {
    painter->setPen(QPen(palette.accentPrimary, 2));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(boundingRect().adjusted(1, 1, -1, -1),
                             CORNER_RADIUS, CORNER_RADIUS);
  }

  // Breakpoint indicator (red circle in top-left corner)
  if (m_hasBreakpoint) {
    const qreal radius = 8.0;
    const QPointF center(radius + 4, radius + 4);

    painter->setBrush(QColor(220, 60, 60)); // Red
    painter->setPen(QPen(QColor(180, 40, 40), 2));
    painter->drawEllipse(center, radius, radius);

    // Inner highlight for 3D effect
    painter->setBrush(QColor(255, 100, 100, 80));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(center - QPointF(2, 2), radius * 0.4, radius * 0.4);
  }

  // Currently executing indicator (pulsing green border + glow)
  if (m_isCurrentlyExecuting) {
    // Outer glow effect
    for (int i = 3; i >= 0; --i) {
      int alpha = 40 - (i * 10);
      QColor glowColor(60, 220, 120, alpha);
      painter->setPen(QPen(glowColor, 3 + i * 2));
      painter->setBrush(Qt::NoBrush);
      painter->drawRoundedRect(boundingRect().adjusted(-i, -i, i, i),
                               CORNER_RADIUS + i, CORNER_RADIUS + i);
    }

    // Solid green border
    painter->setPen(QPen(QColor(60, 220, 120), 3));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(boundingRect().adjusted(1, 1, -1, -1),
                             CORNER_RADIUS, CORNER_RADIUS);

    // Execution arrow indicator in top-right corner
    const qreal arrowSize = 16.0;
    const QPointF arrowCenter(NODE_WIDTH - arrowSize - 4, arrowSize / 2 + 4);

    QPainterPath arrowPath;
    arrowPath.moveTo(arrowCenter + QPointF(-arrowSize / 2, -arrowSize / 3));
    arrowPath.lineTo(arrowCenter + QPointF(arrowSize / 2, 0));
    arrowPath.lineTo(arrowCenter + QPointF(-arrowSize / 2, arrowSize / 3));
    arrowPath.closeSubpath();

    painter->setBrush(QColor(60, 220, 120));
    painter->setPen(QPen(QColor(40, 180, 90), 2));
    painter->drawPath(arrowPath);
  }
}

QVariant NMGraphNodeItem::itemChange(GraphicsItemChange change,
                                     const QVariant &value) {
  if (change == ItemPositionHasChanged) {
    // Update connections when node moves
    // This would be handled by the parent scene
  } else if (change == ItemSelectedHasChanged) {
    m_isSelected = value.toBool();
  }
  return QGraphicsItem::itemChange(change, value);
}

void NMGraphNodeItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
  QMenu menu;

  // Toggle Breakpoint action
  QAction *breakpointAction =
      menu.addAction(m_hasBreakpoint ? "Remove Breakpoint" : "Add Breakpoint");
  breakpointAction->setIcon(
      QIcon::fromTheme(m_hasBreakpoint ? "edit-delete" : "list-add"));

  menu.addSeparator();

  // Edit Node action
  QAction *editAction = menu.addAction("Edit Node Properties");
  editAction->setIcon(QIcon::fromTheme("document-properties"));

  // Delete Node action
  QAction *deleteAction = menu.addAction("Delete Node");
  deleteAction->setIcon(QIcon::fromTheme("edit-delete"));

  // Show menu and handle action
  QAction *selectedAction = menu.exec(event->screenPos());

  if (selectedAction == breakpointAction) {
    // Toggle breakpoint via Play Mode Controller
    NMPlayModeController::instance().toggleBreakpoint(m_nodeIdString);
  } else if (selectedAction == deleteAction) {
    // TODO: Implement node deletion via undo system
    // For now, just mark for deletion
  } else if (selectedAction == editAction) {
    // TODO: Open inspector with node properties
  }

  event->accept();
}

// ============================================================================
// NMGraphConnectionItem
// ============================================================================

NMGraphConnectionItem::NMGraphConnectionItem(NMGraphNodeItem *startNode,
                                             NMGraphNodeItem *endNode)
    : m_startNode(startNode), m_endNode(endNode) {
  setZValue(-1); // Draw behind nodes
  updatePath();
}

void NMGraphConnectionItem::updatePath() {
  if (!m_startNode || !m_endNode)
    return;

  QPointF start = m_startNode->sceneBoundingRect().center();
  start.setX(m_startNode->sceneBoundingRect().right());

  QPointF end = m_endNode->sceneBoundingRect().center();
  end.setX(m_endNode->sceneBoundingRect().left());

  // Create bezier curve
  m_path = QPainterPath();
  m_path.moveTo(start);

  qreal dx = std::abs(end.x() - start.x()) * 0.5;
  m_path.cubicTo(start + QPointF(dx, 0), end + QPointF(-dx, 0), end);

  prepareGeometryChange();
}

QRectF NMGraphConnectionItem::boundingRect() const {
  return m_path.boundingRect().adjusted(-5, -5, 5, 5);
}

void NMGraphConnectionItem::paint(QPainter *painter,
                                  const QStyleOptionGraphicsItem * /*option*/,
                                  QWidget * /*widget*/) {
  const auto &palette = NMStyleManager::instance().palette();

  painter->setRenderHint(QPainter::Antialiasing);
  painter->setPen(QPen(palette.connectionLine, 2));
  painter->setBrush(Qt::NoBrush);
  painter->drawPath(m_path);
}

// ============================================================================
// NMStoryGraphScene
// ============================================================================

NMStoryGraphScene::NMStoryGraphScene(QObject *parent) : QGraphicsScene(parent) {
  setSceneRect(-5000, -5000, 10000, 10000);
}

NMGraphNodeItem *NMStoryGraphScene::addNode(const QString &title,
                                            const QString &nodeType,
                                            const QPointF &pos) {
  auto *node = new NMGraphNodeItem(title, nodeType);
  node->setPos(pos);
  addItem(node);
  m_nodes.append(node);
  return node;
}

NMGraphConnectionItem *NMStoryGraphScene::addConnection(NMGraphNodeItem *from,
                                                        NMGraphNodeItem *to) {
  auto *connection = new NMGraphConnectionItem(from, to);
  addItem(connection);
  m_connections.append(connection);
  return connection;
}

void NMStoryGraphScene::clearGraph() {
  for (auto *conn : m_connections) {
    removeItem(conn);
    delete conn;
  }
  m_connections.clear();

  for (auto *node : m_nodes) {
    removeItem(node);
    delete node;
  }
  m_nodes.clear();
}

void NMStoryGraphScene::removeNode(NMGraphNodeItem *node) {
  if (!node)
    return;

  // Remove all connections attached to this node
  auto connections = findConnectionsForNode(node);
  for (auto *conn : connections) {
    removeConnection(conn);
  }

  // Remove from list and scene
  m_nodes.removeAll(node);
  removeItem(node);
  emit nodeDeleted(node);
  delete node;
}

void NMStoryGraphScene::removeConnection(NMGraphConnectionItem *connection) {
  if (!connection)
    return;

  m_connections.removeAll(connection);
  removeItem(connection);
  emit connectionDeleted(connection);
  delete connection;
}

QList<NMGraphConnectionItem *>
NMStoryGraphScene::findConnectionsForNode(NMGraphNodeItem *node) const {
  QList<NMGraphConnectionItem *> result;
  for (auto *conn : m_connections) {
    if (conn->startNode() == node || conn->endNode() == node) {
      result.append(conn);
    }
  }
  return result;
}

void NMStoryGraphScene::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
    // Delete selected items
    auto selected = selectedItems();
    for (auto *item : selected) {
      if (auto *node = qgraphicsitem_cast<NMGraphNodeItem *>(item)) {
        removeNode(node);
      } else if (auto *conn =
                     qgraphicsitem_cast<NMGraphConnectionItem *>(item)) {
        removeConnection(conn);
      }
    }
    event->accept();
    return;
  }

  QGraphicsScene::keyPressEvent(event);
}

void NMStoryGraphScene::drawBackground(QPainter *painter, const QRectF &rect) {
  const auto &palette = NMStyleManager::instance().palette();

  // Fill background
  painter->fillRect(rect, palette.bgDarkest);

  // Draw grid (dots pattern for graph view)
  painter->setPen(palette.gridLine);

  qreal gridSize = 32.0;
  qreal left = rect.left() - std::fmod(rect.left(), gridSize);
  qreal top = rect.top() - std::fmod(rect.top(), gridSize);

  for (qreal x = left; x < rect.right(); x += gridSize) {
    for (qreal y = top; y < rect.bottom(); y += gridSize) {
      painter->drawPoint(QPointF(x, y));
    }
  }

  // Draw origin
  painter->setPen(QPen(palette.accentPrimary, 1));
  if (rect.left() <= 0 && rect.right() >= 0) {
    painter->drawLine(QLineF(0, rect.top(), 0, rect.bottom()));
  }
  if (rect.top() <= 0 && rect.bottom() >= 0) {
    painter->drawLine(QLineF(rect.left(), 0, rect.right(), 0));
  }
}

// ============================================================================
// NMStoryGraphView
// ============================================================================

NMStoryGraphView::NMStoryGraphView(QWidget *parent) : QGraphicsView(parent) {
  setRenderHint(QPainter::Antialiasing);
  setRenderHint(QPainter::SmoothPixmapTransform);
  setViewportUpdateMode(FullViewportUpdate);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setTransformationAnchor(AnchorUnderMouse);
  setResizeAnchor(AnchorViewCenter);
  setDragMode(RubberBandDrag);
}

void NMStoryGraphView::setConnectionDrawingMode(bool enabled) {
  m_isDrawingConnection = enabled;
  if (!enabled) {
    m_connectionStartNode = nullptr;
  }
  viewport()->update();
}

void NMStoryGraphView::setZoomLevel(qreal zoom) {
  zoom = qBound(0.1, zoom, 5.0);
  if (qFuzzyCompare(m_zoomLevel, zoom))
    return;

  qreal scaleFactor = zoom / m_zoomLevel;
  m_zoomLevel = zoom;

  scale(scaleFactor, scaleFactor);
  emit zoomChanged(m_zoomLevel);
}

void NMStoryGraphView::centerOnGraph() {
  if (scene() && !scene()->items().isEmpty()) {
    centerOn(scene()->itemsBoundingRect().center());
  } else {
    centerOn(0, 0);
  }
}

void NMStoryGraphView::wheelEvent(QWheelEvent *event) {
  qreal factor = 1.15;
  if (event->angleDelta().y() < 0) {
    factor = 1.0 / factor;
  }

  setZoomLevel(m_zoomLevel * factor);
  event->accept();
}

void NMStoryGraphView::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::MiddleButton) {
    m_isPanning = true;
    m_lastPanPoint = event->pos();
    setCursor(Qt::ClosedHandCursor);
    event->accept();
    return;
  }

  // Ctrl+LeftClick to start drawing a connection
  if (event->button() == Qt::LeftButton &&
      event->modifiers() & Qt::ControlModifier) {
    QPointF scenePos = mapToScene(event->pos());
    auto *item = scene()->itemAt(scenePos, transform());
    if (auto *node = qgraphicsitem_cast<NMGraphNodeItem *>(item)) {
      m_isDrawingConnection = true;
      m_connectionStartNode = node;
      m_connectionEndPoint = scenePos;
      setCursor(Qt::CrossCursor);
      event->accept();
      return;
    }
  }

  QGraphicsView::mousePressEvent(event);
}

void NMStoryGraphView::mouseMoveEvent(QMouseEvent *event) {
  if (m_isPanning) {
    QPoint delta = event->pos() - m_lastPanPoint;
    m_lastPanPoint = event->pos();

    horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
    verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
    event->accept();
    return;
  }

  // Update connection drawing line
  if (m_isDrawingConnection && m_connectionStartNode) {
    m_connectionEndPoint = mapToScene(event->pos());
    viewport()->update();
    event->accept();
    return;
  }

  QGraphicsView::mouseMoveEvent(event);
}

void NMStoryGraphView::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::MiddleButton && m_isPanning) {
    m_isPanning = false;
    setCursor(Qt::ArrowCursor);
    event->accept();
    return;
  }

  // Finish drawing connection
  if (event->button() == Qt::LeftButton && m_isDrawingConnection &&
      m_connectionStartNode) {
    QPointF scenePos = mapToScene(event->pos());
    auto *item = scene()->itemAt(scenePos, transform());
    if (auto *endNode = qgraphicsitem_cast<NMGraphNodeItem *>(item)) {
      if (endNode != m_connectionStartNode) {
        // Emit signal to create connection
        emit requestConnection(m_connectionStartNode, endNode);
      }
    }

    m_isDrawingConnection = false;
    m_connectionStartNode = nullptr;
    setCursor(Qt::ArrowCursor);
    viewport()->update();
    event->accept();
    return;
  }

  QGraphicsView::mouseReleaseEvent(event);
}

void NMStoryGraphView::drawForeground(QPainter *painter,
                                      const QRectF & /*rect*/) {
  // Draw connection line being created
  if (m_isDrawingConnection && m_connectionStartNode) {
    const auto &palette = NMStyleManager::instance().palette();

    QPointF start = m_connectionStartNode->sceneBoundingRect().center();
    start.setX(m_connectionStartNode->sceneBoundingRect().right());
    QPointF end = m_connectionEndPoint;

    // Draw bezier curve
    QPainterPath path;
    path.moveTo(start);

    qreal dx = std::abs(end.x() - start.x()) * 0.5;
    path.cubicTo(start + QPointF(dx, 0), end + QPointF(-dx, 0), end);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(palette.accentPrimary, 2, Qt::DashLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path);
  }
}

// ============================================================================
// NMNodePalette
// ============================================================================

NMNodePalette::NMNodePalette(QWidget *parent) : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(4, 4, 4, 4);
  layout->setSpacing(4);

  const auto &palette = NMStyleManager::instance().palette();

  // Title
  auto *titleLabel = new QLabel(tr("Create Node"), this);
  titleLabel->setStyleSheet(
      QString("color: %1; font-weight: bold; padding: 4px;")
          .arg(palette.textPrimary.name()));
  layout->addWidget(titleLabel);

  // Separator
  auto *separator = new QFrame(this);
  separator->setFrameShape(QFrame::HLine);
  separator->setStyleSheet(
      QString("background-color: %1;").arg(palette.borderDark.name()));
  layout->addWidget(separator);

  // Node type buttons
  createNodeButton("Entry", "▶");
  createNodeButton("Dialogue", "💬");
  createNodeButton("Choice", "⚑");
  createNodeButton("Scene", "🎬");
  createNodeButton("Label", "🏷");
  createNodeButton("Script", "⚙");

  layout->addStretch();

  // Style the widget
  setStyleSheet(QString("QWidget { background-color: %1; border: 1px solid %2; "
                        "border-radius: 4px; }")
                    .arg(palette.bgDark.name())
                    .arg(palette.borderDark.name()));
  setMinimumWidth(120);
  setMaximumWidth(150);
}

void NMNodePalette::createNodeButton(const QString &nodeType,
                                     const QString &icon) {
  auto *layout = qobject_cast<QVBoxLayout *>(this->layout());
  if (!layout)
    return;

  auto *button = new QPushButton(QString("%1 %2").arg(icon, nodeType), this);
  button->setMinimumHeight(32);

  const auto &palette = NMStyleManager::instance().palette();
  button->setStyleSheet(QString("QPushButton {"
                                "  background-color: %1;"
                                "  color: %2;"
                                "  border: 1px solid %3;"
                                "  border-radius: 4px;"
                                "  padding: 6px 12px;"
                                "  text-align: left;"
                                "}"
                                "QPushButton:hover {"
                                "  background-color: %4;"
                                "  border-color: %5;"
                                "}"
                                "QPushButton:pressed {"
                                "  background-color: %6;"
                                "}")
                            .arg(palette.bgMedium.name())
                            .arg(palette.textPrimary.name())
                            .arg(palette.borderDark.name())
                            .arg(palette.bgLight.name())
                            .arg(palette.accentPrimary.name())
                            .arg(palette.bgDark.name()));

  connect(button, &QPushButton::clicked, this,
          [this, nodeType]() { emit nodeTypeSelected(nodeType); });

  layout->insertWidget(layout->count() - 1, button);
}

// ============================================================================
// NMStoryGraphPanel
// ============================================================================

NMStoryGraphPanel::NMStoryGraphPanel(QWidget *parent)
    : NMDockPanel(tr("Story Graph"), parent) {
  setPanelId("StoryGraph");
  setupContent();
  setupToolBar();
  setupNodePalette();
}

NMStoryGraphPanel::~NMStoryGraphPanel() = default;

void NMStoryGraphPanel::onInitialize() {
  // Load demo data for testing
  loadDemoGraph();

  if (m_view) {
    m_view->centerOnGraph();
  }

  // Connect to Play Mode Controller signals
  auto &playController = NMPlayModeController::instance();
  connect(&playController, &NMPlayModeController::currentNodeChanged, this,
          &NMStoryGraphPanel::onCurrentNodeChanged);
  connect(&playController, &NMPlayModeController::breakpointsChanged, this,
          &NMStoryGraphPanel::onBreakpointsChanged);
}

void NMStoryGraphPanel::onUpdate(double /*deltaTime*/) {
  // Update connections when nodes move
  // For now, this is handled reactively
}

void NMStoryGraphPanel::loadDemoGraph() {
  if (!m_scene)
    return;

  m_scene->clearGraph();

  // Create demo nodes
  auto *startNode = m_scene->addNode("Start", "Entry", QPointF(0, 0));
  startNode->setNodeId(1);
  startNode->setNodeIdString("node_start");

  auto *dialogueNode =
      m_scene->addNode("Welcome to the game!", "Dialogue", QPointF(300, 0));
  dialogueNode->setNodeId(2);
  dialogueNode->setNodeIdString("node_dialogue_1");

  auto *choiceNode =
      m_scene->addNode("What will you do?", "Choice", QPointF(600, 0));
  choiceNode->setNodeId(3);
  choiceNode->setNodeIdString("node_choice_1");

  auto *branch1 = m_scene->addNode("You explore the forest", "Dialogue",
                                   QPointF(900, -100));
  branch1->setNodeId(4);
  branch1->setNodeIdString("node_branch_forest");

  auto *branch2 = m_scene->addNode("You head to the village", "Dialogue",
                                   QPointF(900, 100));
  branch2->setNodeId(5);
  branch2->setNodeIdString("node_branch_village");

  // Create connections
  m_scene->addConnection(startNode, dialogueNode);
  m_scene->addConnection(dialogueNode, choiceNode);
  m_scene->addConnection(choiceNode, branch1);
  m_scene->addConnection(choiceNode, branch2);
}

void NMStoryGraphPanel::setupToolBar() {
  m_toolBar = new QToolBar(this);
  m_toolBar->setObjectName("StoryGraphToolBar");
  m_toolBar->setIconSize(QSize(16, 16));

  QAction *actionZoomIn = m_toolBar->addAction(tr("+"));
  actionZoomIn->setToolTip(tr("Zoom In"));
  connect(actionZoomIn, &QAction::triggered, this,
          &NMStoryGraphPanel::onZoomIn);

  QAction *actionZoomOut = m_toolBar->addAction(tr("-"));
  actionZoomOut->setToolTip(tr("Zoom Out"));
  connect(actionZoomOut, &QAction::triggered, this,
          &NMStoryGraphPanel::onZoomOut);

  QAction *actionZoomReset = m_toolBar->addAction(tr("1:1"));
  actionZoomReset->setToolTip(tr("Reset Zoom"));
  connect(actionZoomReset, &QAction::triggered, this,
          &NMStoryGraphPanel::onZoomReset);

  QAction *actionFit = m_toolBar->addAction(tr("Fit"));
  actionFit->setToolTip(tr("Fit Graph to View"));
  connect(actionFit, &QAction::triggered, this,
          &NMStoryGraphPanel::onFitToGraph);

  if (auto *layout = qobject_cast<QVBoxLayout *>(m_contentWidget->layout())) {
    layout->insertWidget(0, m_toolBar);
  }
}

void NMStoryGraphPanel::setupContent() {
  m_contentWidget = new QWidget(this);
  auto *mainLayout = new QVBoxLayout(m_contentWidget);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // Create horizontal layout for node palette + graph view
  auto *hLayout = new QHBoxLayout();
  hLayout->setContentsMargins(0, 0, 0, 0);
  hLayout->setSpacing(4);

  m_scene = new NMStoryGraphScene(this);
  m_view = new NMStoryGraphView(m_contentWidget);
  m_view->setScene(m_scene);

  hLayout->addWidget(m_view, 1); // Graph view takes most space

  mainLayout->addLayout(hLayout);

  setContentWidget(m_contentWidget);

  // Connect view signals
  connect(m_view, &NMStoryGraphView::requestConnection, this,
          &NMStoryGraphPanel::onRequestConnection);
}

void NMStoryGraphPanel::setupNodePalette() {
  if (!m_contentWidget)
    return;

  // Find the horizontal layout
  auto *mainLayout = qobject_cast<QVBoxLayout *>(m_contentWidget->layout());
  if (!mainLayout)
    return;

  QHBoxLayout *hLayout = nullptr;
  for (int i = 0; i < mainLayout->count(); ++i) {
    hLayout = qobject_cast<QHBoxLayout *>(mainLayout->itemAt(i)->layout());
    if (hLayout)
      break;
  }

  if (!hLayout)
    return;

  // Create and add node palette
  m_nodePalette = new NMNodePalette(m_contentWidget);
  hLayout->insertWidget(0, m_nodePalette); // Add to left side

  // Connect signals
  connect(m_nodePalette, &NMNodePalette::nodeTypeSelected, this,
          &NMStoryGraphPanel::onNodeTypeSelected);
}

void NMStoryGraphPanel::onZoomIn() {
  if (m_view) {
    m_view->setZoomLevel(m_view->zoomLevel() * 1.25);
  }
}

void NMStoryGraphPanel::onZoomOut() {
  if (m_view) {
    m_view->setZoomLevel(m_view->zoomLevel() / 1.25);
  }
}

void NMStoryGraphPanel::onZoomReset() {
  if (m_view) {
    m_view->setZoomLevel(1.0);
    m_view->centerOnGraph();
  }
}

void NMStoryGraphPanel::onFitToGraph() {
  if (m_view && m_scene && !m_scene->items().isEmpty()) {
    m_view->fitInView(m_scene->itemsBoundingRect().adjusted(-50, -50, 50, 50),
                      Qt::KeepAspectRatio);
  }
}

void NMStoryGraphPanel::onCurrentNodeChanged(const QString &nodeId) {
  updateCurrentNode(nodeId);
}

void NMStoryGraphPanel::onBreakpointsChanged() { updateNodeBreakpoints(); }

NMGraphNodeItem *
NMStoryGraphPanel::findNodeByIdString(const QString &id) const {
  if (!m_scene)
    return nullptr;

  const auto items = m_scene->items();
  for (auto *item : items) {
    if (auto *node = qgraphicsitem_cast<NMGraphNodeItem *>(item)) {
      if (node->nodeIdString() == id) {
        return node;
      }
    }
  }
  return nullptr;
}

void NMStoryGraphPanel::updateNodeBreakpoints() {
  if (!m_scene)
    return;

  auto &playController = NMPlayModeController::instance();
  const QSet<QString> breakpoints = playController.breakpoints();

  // Update all nodes
  const auto items = m_scene->items();
  for (auto *item : items) {
    if (auto *node = qgraphicsitem_cast<NMGraphNodeItem *>(item)) {
      bool hasBreakpoint = breakpoints.contains(node->nodeIdString());
      node->setBreakpoint(hasBreakpoint);
    }
  }
}

void NMStoryGraphPanel::updateCurrentNode(const QString &nodeId) {
  if (!m_scene)
    return;

  // Clear previous execution state
  if (!m_currentExecutingNode.isEmpty()) {
    if (auto *prevNode = findNodeByIdString(m_currentExecutingNode)) {
      prevNode->setCurrentlyExecuting(false);
    }
  }

  // Set new execution state
  m_currentExecutingNode = nodeId;
  if (!nodeId.isEmpty()) {
    if (auto *currentNode = findNodeByIdString(nodeId)) {
      currentNode->setCurrentlyExecuting(true);

      // Center view on executing node
      if (m_view) {
        m_view->centerOn(currentNode);
      }
    }
  }
}

void NMStoryGraphPanel::createNode(const QString &nodeType) {
  if (!m_scene || !m_view)
    return;

  // Get center of visible area
  QPointF centerPos = m_view->mapToScene(m_view->viewport()->rect().center());

  // Create node with unique ID
  QString nodeId =
      QString("node_%1_%2").arg(nodeType.toLower()).arg(m_nextNodeId++);
  auto *node =
      m_scene->addNode(QString("New %1").arg(nodeType), nodeType, centerPos);
  node->setNodeId(m_nextNodeId);
  node->setNodeIdString(nodeId);

  // Select the new node
  m_scene->clearSelection();
  node->setSelected(true);
}

void NMStoryGraphPanel::onNodeTypeSelected(const QString &nodeType) {
  createNode(nodeType);
}

void NMStoryGraphPanel::onRequestConnection(NMGraphNodeItem *from,
                                            NMGraphNodeItem *to) {
  if (!m_scene || !from || !to)
    return;

  // Check if connection already exists
  for (const auto *conn : m_scene->connections()) {
    if (conn->startNode() == from && conn->endNode() == to) {
      return; // Connection already exists
    }
  }

  // Create the connection
  m_scene->addConnection(from, to);
}

void NMStoryGraphPanel::onDeleteSelected() {
  if (!m_scene)
    return;

  auto selected = m_scene->selectedItems();
  for (auto *item : selected) {
    if (auto *node = qgraphicsitem_cast<NMGraphNodeItem *>(item)) {
      m_scene->removeNode(node);
    } else if (auto *conn = qgraphicsitem_cast<NMGraphConnectionItem *>(item)) {
      m_scene->removeConnection(conn);
    }
  }
}

} // namespace NovelMind::editor::qt
