#pragma once

/**
 * @file nm_hierarchy_panel.hpp
 * @brief Hierarchy panel for scene object tree view
 *
 * Displays the scene hierarchy as a tree:
 * - Scene layers
 * - Objects with parent-child relationships
 * - Selection synchronization
 * - Drag-and-drop (Phase 2+)
 */

#include "NovelMind/editor/qt/nm_dock_panel.hpp"
#include <QToolBar>
#include <QTreeWidget>

namespace NovelMind::editor::qt {

/**
 * @brief Tree widget for scene hierarchy
 */
class NMHierarchyTree : public QTreeWidget {
  Q_OBJECT

public:
  explicit NMHierarchyTree(QWidget *parent = nullptr);

  /**
   * @brief Clear and rebuild the tree
   */
  void refresh();

  /**
   * @brief Add a demo hierarchy for testing
   */
  void loadDemoHierarchy();

signals:
  void itemSelected(const QString &objectId);
  void itemDoubleClicked(const QString &objectId);

protected:
  void selectionChanged(const QItemSelection &selected,
                        const QItemSelection &deselected) override;

private slots:
  void onItemDoubleClicked(QTreeWidgetItem *item, int column);
};

/**
 * @brief Hierarchy panel for scene structure
 */
class NMHierarchyPanel : public NMDockPanel {
  Q_OBJECT

public:
  explicit NMHierarchyPanel(QWidget *parent = nullptr);
  ~NMHierarchyPanel() override;

  void onInitialize() override;
  void onUpdate(double deltaTime) override;

  [[nodiscard]] NMHierarchyTree *hierarchyTree() const { return m_tree; }

  /**
   * @brief Refresh the hierarchy display
   */
  void refresh();

  /**
   * @brief Select an item by object ID
   */
  void selectObject(const QString &objectId);

signals:
  void objectSelected(const QString &objectId);
  void objectDoubleClicked(const QString &objectId);

private slots:
  void onRefresh();
  void onExpandAll();
  void onCollapseAll();

private:
  void setupToolBar();
  void setupContent();

  NMHierarchyTree *m_tree = nullptr;
  QWidget *m_contentWidget = nullptr;
  QToolBar *m_toolBar = nullptr;
};

} // namespace NovelMind::editor::qt
