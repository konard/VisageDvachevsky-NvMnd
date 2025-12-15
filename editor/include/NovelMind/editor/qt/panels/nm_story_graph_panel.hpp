#pragma once

/**
 * @file nm_story_graph_panel.hpp
 * @brief Story Graph panel for node-based visual scripting
 *
 * Displays the story graph with:
 * - Node representation
 * - Connection lines
 * - Mini-map
 * - Viewport controls
 */

#include "NovelMind/editor/qt/nm_dock_panel.hpp"
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QToolBar>

namespace NovelMind::editor::qt {

/**
 * @brief Graphics item representing a story graph node
 */
class NMGraphNodeItem : public QGraphicsItem {
public:
  explicit NMGraphNodeItem(const QString &title, const QString &nodeType);

  void setTitle(const QString &title);
  [[nodiscard]] QString title() const { return m_title; }

  void setNodeType(const QString &type);
  [[nodiscard]] QString nodeType() const { return m_nodeType; }

  void setNodeId(uint64_t id) { m_nodeId = id; }
  [[nodiscard]] uint64_t nodeId() const { return m_nodeId; }

  void setNodeIdString(const QString &id) { m_nodeIdString = id; }
  [[nodiscard]] QString nodeIdString() const { return m_nodeIdString; }

  void setSelected(bool selected);
  void setBreakpoint(bool hasBreakpoint);
  void setCurrentlyExecuting(bool isExecuting);

  [[nodiscard]] bool hasBreakpoint() const { return m_hasBreakpoint; }
  [[nodiscard]] bool isCurrentlyExecuting() const {
    return m_isCurrentlyExecuting;
  }

  QRectF boundingRect() const override;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;

protected:
  QVariant itemChange(GraphicsItemChange change,
                      const QVariant &value) override;
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
  QString m_title;
  QString m_nodeType;
  uint64_t m_nodeId = 0;
  QString m_nodeIdString;
  bool m_isSelected = false;
  bool m_hasBreakpoint = false;
  bool m_isCurrentlyExecuting = false;

  static constexpr qreal NODE_WIDTH = 200;
  static constexpr qreal NODE_HEIGHT = 80;
  static constexpr qreal CORNER_RADIUS = 8;
};

/**
 * @brief Graphics item representing a connection between nodes
 */
class NMGraphConnectionItem : public QGraphicsItem {
public:
  NMGraphConnectionItem(NMGraphNodeItem *startNode, NMGraphNodeItem *endNode);

  void updatePath();

  [[nodiscard]] NMGraphNodeItem *startNode() const { return m_startNode; }
  [[nodiscard]] NMGraphNodeItem *endNode() const { return m_endNode; }

  QRectF boundingRect() const override;
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;

private:
  NMGraphNodeItem *m_startNode;
  NMGraphNodeItem *m_endNode;
  QPainterPath m_path;
};

/**
 * @brief Graphics scene for the story graph
 */
class NMStoryGraphScene : public QGraphicsScene {
  Q_OBJECT

public:
  explicit NMStoryGraphScene(QObject *parent = nullptr);

  /**
   * @brief Add a node to the graph
   */
  NMGraphNodeItem *addNode(const QString &title, const QString &nodeType,
                           const QPointF &pos);

  /**
   * @brief Add a connection between nodes
   */
  NMGraphConnectionItem *addConnection(NMGraphNodeItem *from,
                                       NMGraphNodeItem *to);

  /**
   * @brief Remove a node and its connections
   */
  void removeNode(NMGraphNodeItem *node);

  /**
   * @brief Remove a connection
   */
  void removeConnection(NMGraphConnectionItem *connection);

  /**
   * @brief Clear all nodes and connections
   */
  void clearGraph();

  /**
   * @brief Get all nodes
   */
  [[nodiscard]] const QList<NMGraphNodeItem *> &nodes() const {
    return m_nodes;
  }

  /**
   * @brief Get all connections
   */
  [[nodiscard]] const QList<NMGraphConnectionItem *> &connections() const {
    return m_connections;
  }

  /**
   * @brief Find connections attached to a node
   */
  QList<NMGraphConnectionItem *>
  findConnectionsForNode(NMGraphNodeItem *node) const;

signals:
  void nodeDeleted(NMGraphNodeItem *node);
  void connectionAdded(NMGraphConnectionItem *connection);
  void connectionDeleted(NMGraphConnectionItem *connection);

protected:
  void drawBackground(QPainter *painter, const QRectF &rect) override;
  void keyPressEvent(QKeyEvent *event) override;

private:
  QList<NMGraphNodeItem *> m_nodes;
  QList<NMGraphConnectionItem *> m_connections;
};

/**
 * @brief Graphics view for story graph with pan/zoom
 */
class NMStoryGraphView : public QGraphicsView {
  Q_OBJECT

public:
  explicit NMStoryGraphView(QWidget *parent = nullptr);

  void setZoomLevel(qreal zoom);
  [[nodiscard]] qreal zoomLevel() const { return m_zoomLevel; }

  void centerOnGraph();

  void setConnectionDrawingMode(bool enabled);
  [[nodiscard]] bool isConnectionDrawingMode() const {
    return m_isDrawingConnection;
  }

signals:
  void zoomChanged(qreal newZoom);
  void nodeClicked(NMGraphNodeItem *node);
  void requestConnection(NMGraphNodeItem *from, NMGraphNodeItem *to);

protected:
  void wheelEvent(QWheelEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void drawForeground(QPainter *painter, const QRectF &rect) override;

private:
  qreal m_zoomLevel = 1.0;
  bool m_isPanning = false;
  QPoint m_lastPanPoint;
  bool m_isDrawingConnection = false;
  NMGraphNodeItem *m_connectionStartNode = nullptr;
  QPointF m_connectionEndPoint;
};

/**
 * @brief Node creation palette for adding new nodes to the graph
 */
class NMNodePalette : public QWidget {
  Q_OBJECT

public:
  explicit NMNodePalette(QWidget *parent = nullptr);

signals:
  void nodeTypeSelected(const QString &nodeType);

private:
  void createNodeButton(const QString &nodeType, const QString &icon);
};

/**
 * @brief Story Graph panel for visual scripting
 */
class NMStoryGraphPanel : public NMDockPanel {
  Q_OBJECT

public:
  explicit NMStoryGraphPanel(QWidget *parent = nullptr);
  ~NMStoryGraphPanel() override;

  void onInitialize() override;
  void onUpdate(double deltaTime) override;

  [[nodiscard]] NMStoryGraphScene *graphScene() const { return m_scene; }
  [[nodiscard]] NMStoryGraphView *graphView() const { return m_view; }

  /**
   * @brief Load demo data for testing
   */
  void loadDemoGraph();

  /**
   * @brief Find node by string ID
   */
  NMGraphNodeItem *findNodeByIdString(const QString &id) const;

  /**
   * @brief Create a new node at the view center
   */
  void createNode(const QString &nodeType);

signals:
  void nodeSelected(uint64_t nodeId);
  void nodeDoubleClicked(uint64_t nodeId);

private slots:
  void onZoomIn();
  void onZoomOut();
  void onZoomReset();
  void onFitToGraph();
  void onCurrentNodeChanged(const QString &nodeId);
  void onBreakpointsChanged();
  void onNodeTypeSelected(const QString &nodeType);
  void onRequestConnection(NMGraphNodeItem *from, NMGraphNodeItem *to);
  void onDeleteSelected();

private:
  void setupToolBar();
  void setupContent();
  void setupNodePalette();
  void updateNodeBreakpoints();
  void updateCurrentNode(const QString &nodeId);

  NMStoryGraphScene *m_scene = nullptr;
  NMStoryGraphView *m_view = nullptr;
  QWidget *m_contentWidget = nullptr;
  QToolBar *m_toolBar = nullptr;
  NMNodePalette *m_nodePalette = nullptr;
  QString m_currentExecutingNode;
  uint64_t m_nextNodeId = 1000;
};

} // namespace NovelMind::editor::qt
