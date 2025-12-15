#include "NovelMind/editor/qt/panels/nm_hierarchy_panel.hpp"
#include "NovelMind/editor/qt/nm_style_manager.hpp"

#include <QAction>
#include <QHeaderView>
#include <QToolBar>
#include <QVBoxLayout>

namespace NovelMind::editor::qt {

// ============================================================================
// NMHierarchyTree
// ============================================================================

NMHierarchyTree::NMHierarchyTree(QWidget *parent) : QTreeWidget(parent) {
  setHeaderHidden(true);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setDragEnabled(false); // Phase 2: Enable for drag-drop
  setAcceptDrops(false);
  setDropIndicatorShown(true);
  setAnimated(true);
  setIndentation(16);

  connect(this, &QTreeWidget::itemDoubleClicked, this,
          &NMHierarchyTree::onItemDoubleClicked);
}

void NMHierarchyTree::refresh() {
  // This would be connected to the actual scene graph
  // For now, load demo data
  loadDemoHierarchy();
}

void NMHierarchyTree::loadDemoHierarchy() {
  clear();

  // Create demo hierarchy matching a visual novel scene structure
  auto *rootItem = new QTreeWidgetItem(this);
  rootItem->setText(0, "Scene: MainMenu");
  rootItem->setData(0, Qt::UserRole, "scene_mainmenu");
  rootItem->setExpanded(true);

  // Background layer
  auto *bgLayer = new QTreeWidgetItem(rootItem);
  bgLayer->setText(0, "Background Layer");
  bgLayer->setData(0, Qt::UserRole, "layer_bg");
  bgLayer->setExpanded(true);

  auto *bgImage = new QTreeWidgetItem(bgLayer);
  bgImage->setText(0, "Background Image");
  bgImage->setData(0, Qt::UserRole, "obj_bg_image");

  // Character layer
  auto *charLayer = new QTreeWidgetItem(rootItem);
  charLayer->setText(0, "Character Layer");
  charLayer->setData(0, Qt::UserRole, "layer_char");
  charLayer->setExpanded(true);

  auto *char1 = new QTreeWidgetItem(charLayer);
  char1->setText(0, "Character: Alice");
  char1->setData(0, Qt::UserRole, "obj_char_alice");

  auto *char2 = new QTreeWidgetItem(charLayer);
  char2->setText(0, "Character: Bob");
  char2->setData(0, Qt::UserRole, "obj_char_bob");

  // UI layer
  auto *uiLayer = new QTreeWidgetItem(rootItem);
  uiLayer->setText(0, "UI Layer");
  uiLayer->setData(0, Qt::UserRole, "layer_ui");
  uiLayer->setExpanded(true);

  auto *dialogueBox = new QTreeWidgetItem(uiLayer);
  dialogueBox->setText(0, "Dialogue Box");
  dialogueBox->setData(0, Qt::UserRole, "obj_dialogue_box");

  auto *nameLabel = new QTreeWidgetItem(dialogueBox);
  nameLabel->setText(0, "Name Label");
  nameLabel->setData(0, Qt::UserRole, "obj_name_label");

  auto *textArea = new QTreeWidgetItem(dialogueBox);
  textArea->setText(0, "Text Area");
  textArea->setData(0, Qt::UserRole, "obj_text_area");

  auto *choiceMenu = new QTreeWidgetItem(uiLayer);
  choiceMenu->setText(0, "Choice Menu");
  choiceMenu->setData(0, Qt::UserRole, "obj_choice_menu");

  // Effect layer
  auto *effectLayer = new QTreeWidgetItem(rootItem);
  effectLayer->setText(0, "Effect Layer");
  effectLayer->setData(0, Qt::UserRole, "layer_effect");

  auto *fadeOverlay = new QTreeWidgetItem(effectLayer);
  fadeOverlay->setText(0, "Fade Overlay");
  fadeOverlay->setData(0, Qt::UserRole, "obj_fade_overlay");
}

void NMHierarchyTree::selectionChanged(const QItemSelection &selected,
                                       const QItemSelection &deselected) {
  QTreeWidget::selectionChanged(selected, deselected);

  QList<QTreeWidgetItem *> selectedItems = this->selectedItems();
  if (!selectedItems.isEmpty()) {
    QString objectId = selectedItems.first()->data(0, Qt::UserRole).toString();
    emit itemSelected(objectId);
  }
}

void NMHierarchyTree::onItemDoubleClicked(QTreeWidgetItem *item,
                                          int /*column*/) {
  if (item) {
    QString objectId = item->data(0, Qt::UserRole).toString();
    emit itemDoubleClicked(objectId);
  }
}

// ============================================================================
// NMHierarchyPanel
// ============================================================================

NMHierarchyPanel::NMHierarchyPanel(QWidget *parent)
    : NMDockPanel(tr("Hierarchy"), parent) {
  setPanelId("Hierarchy");
  setupContent();
  setupToolBar();
}

NMHierarchyPanel::~NMHierarchyPanel() = default;

void NMHierarchyPanel::onInitialize() { refresh(); }

void NMHierarchyPanel::onUpdate(double /*deltaTime*/) {
  // No continuous update needed
}

void NMHierarchyPanel::refresh() {
  if (m_tree) {
    m_tree->refresh();
  }
}

void NMHierarchyPanel::selectObject(const QString &objectId) {
  if (!m_tree)
    return;

  // Find and select the item with the given object ID
  QTreeWidgetItemIterator it(m_tree);
  while (*it) {
    if ((*it)->data(0, Qt::UserRole).toString() == objectId) {
      m_tree->clearSelection();
      (*it)->setSelected(true);
      m_tree->scrollToItem(*it);
      break;
    }
    ++it;
  }
}

void NMHierarchyPanel::setupToolBar() {
  m_toolBar = new QToolBar(this);
  m_toolBar->setObjectName("HierarchyToolBar");
  m_toolBar->setIconSize(QSize(16, 16));

  QAction *actionRefresh = m_toolBar->addAction(tr("Refresh"));
  actionRefresh->setToolTip(tr("Refresh Hierarchy"));
  connect(actionRefresh, &QAction::triggered, this,
          &NMHierarchyPanel::onRefresh);

  m_toolBar->addSeparator();

  QAction *actionExpandAll = m_toolBar->addAction(tr("Expand All"));
  actionExpandAll->setToolTip(tr("Expand All Items"));
  connect(actionExpandAll, &QAction::triggered, this,
          &NMHierarchyPanel::onExpandAll);

  QAction *actionCollapseAll = m_toolBar->addAction(tr("Collapse All"));
  actionCollapseAll->setToolTip(tr("Collapse All Items"));
  connect(actionCollapseAll, &QAction::triggered, this,
          &NMHierarchyPanel::onCollapseAll);

  if (auto *layout = qobject_cast<QVBoxLayout *>(m_contentWidget->layout())) {
    layout->insertWidget(0, m_toolBar);
  }
}

void NMHierarchyPanel::setupContent() {
  m_contentWidget = new QWidget(this);
  auto *layout = new QVBoxLayout(m_contentWidget);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  m_tree = new NMHierarchyTree(m_contentWidget);

  connect(m_tree, &NMHierarchyTree::itemSelected, this,
          &NMHierarchyPanel::objectSelected);
  connect(m_tree, &NMHierarchyTree::itemDoubleClicked, this,
          &NMHierarchyPanel::objectDoubleClicked);

  layout->addWidget(m_tree);

  setContentWidget(m_contentWidget);
}

void NMHierarchyPanel::onRefresh() { refresh(); }

void NMHierarchyPanel::onExpandAll() {
  if (m_tree) {
    m_tree->expandAll();
  }
}

void NMHierarchyPanel::onCollapseAll() {
  if (m_tree) {
    m_tree->collapseAll();
  }
}

} // namespace NovelMind::editor::qt
