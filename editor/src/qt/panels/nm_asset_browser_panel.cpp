#include "NovelMind/editor/qt/panels/nm_asset_browser_panel.hpp"
#include "NovelMind/editor/qt/nm_style_manager.hpp"

#include <QAction>
#include <QDir>
#include <QHeaderView>
#include <QMenu>
#include <QToolBar>
#include <QVBoxLayout>

namespace NovelMind::editor::qt {

NMAssetBrowserPanel::NMAssetBrowserPanel(QWidget *parent)
    : NMDockPanel(tr("Asset Browser"), parent) {
  setPanelId("AssetBrowser");
  setupContent();
  setupToolBar();
}

NMAssetBrowserPanel::~NMAssetBrowserPanel() = default;

void NMAssetBrowserPanel::onInitialize() {
  // Set default root to home directory
  setRootPath(QDir::homePath());
}

void NMAssetBrowserPanel::onUpdate(double /*deltaTime*/) {
  // No continuous update needed
}

void NMAssetBrowserPanel::setRootPath(const QString &path) {
  m_rootPath = path;
  m_currentPath = path;

  if (m_treeModel) {
    m_treeModel->setRootPath(path);
    m_treeView->setRootIndex(m_treeModel->index(path));
  }

  if (m_listModel) {
    m_listModel->setRootPath(path);
    m_listView->setRootIndex(m_listModel->index(path));
  }
}

QString NMAssetBrowserPanel::selectedAssetPath() const {
  if (m_listView && m_listModel) {
    QModelIndex index = m_listView->currentIndex();
    if (index.isValid()) {
      return m_listModel->filePath(index);
    }
  }
  return QString();
}

void NMAssetBrowserPanel::refresh() {
  if (m_listModel && !m_currentPath.isEmpty()) {
    m_listView->setRootIndex(m_listModel->index(m_currentPath));
  }
}

void NMAssetBrowserPanel::setupToolBar() {
  m_toolBar = new QToolBar(this);
  m_toolBar->setObjectName("AssetBrowserToolBar");
  m_toolBar->setIconSize(QSize(16, 16));

  QAction *actionRefresh = m_toolBar->addAction(tr("Refresh"));
  actionRefresh->setToolTip(tr("Refresh Asset List"));
  connect(actionRefresh, &QAction::triggered, this,
          &NMAssetBrowserPanel::refresh);

  m_toolBar->addSeparator();

  QAction *actionImport = m_toolBar->addAction(tr("Import"));
  actionImport->setToolTip(tr("Import Assets"));
  // TODO: Connect to import functionality

  if (auto *layout = qobject_cast<QVBoxLayout *>(m_contentWidget->layout())) {
    layout->insertWidget(0, m_toolBar);
  }
}

void NMAssetBrowserPanel::setupContent() {
  m_contentWidget = new QWidget(this);
  auto *layout = new QVBoxLayout(m_contentWidget);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  // Create splitter for tree/list view
  m_splitter = new QSplitter(Qt::Horizontal, m_contentWidget);

  // Tree view for directory navigation
  m_treeModel = new QFileSystemModel(this);
  m_treeModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

  m_treeView = new QTreeView(m_splitter);
  m_treeView->setModel(m_treeModel);
  m_treeView->setHeaderHidden(true);
  // Hide all columns except name
  for (int i = 1; i < m_treeModel->columnCount(); ++i) {
    m_treeView->hideColumn(i);
  }
  m_treeView->setAnimated(true);
  m_treeView->setIndentation(20);

  connect(m_treeView, &QTreeView::clicked, this,
          &NMAssetBrowserPanel::onTreeClicked);

  m_splitter->addWidget(m_treeView);

  // List view for asset display
  m_listModel = new QFileSystemModel(this);
  m_listModel->setFilter(QDir::Files | QDir::NoDotAndDotDot);
  // Filter for asset types
  m_listModel->setNameFilters(
      {"*.png", "*.jpg", "*.jpeg", "*.bmp", "*.gif", "*.wav", "*.mp3", "*.ogg",
       "*.json", "*.xml", "*.yaml", "*.ttf", "*.otf", "*.nms", "*.nmscene"});
  m_listModel->setNameFilterDisables(false);

  m_listView = new QListView(m_splitter);
  m_listView->setModel(m_listModel);
  m_listView->setViewMode(QListView::IconMode);
  m_listView->setIconSize(QSize(64, 64));
  m_listView->setGridSize(QSize(80, 80));
  m_listView->setSpacing(4);
  m_listView->setResizeMode(QListView::Adjust);
  m_listView->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(m_listView, &QListView::doubleClicked, this,
          &NMAssetBrowserPanel::onListDoubleClicked);
  connect(m_listView, &QListView::customContextMenuRequested, this,
          &NMAssetBrowserPanel::onListContextMenu);

  m_splitter->addWidget(m_listView);

  // Set splitter sizes (30% tree, 70% list)
  m_splitter->setSizes({200, 500});

  layout->addWidget(m_splitter);

  setContentWidget(m_contentWidget);
}

void NMAssetBrowserPanel::onTreeClicked(const QModelIndex &index) {
  if (!index.isValid())
    return;

  QString path = m_treeModel->filePath(index);
  m_currentPath = path;

  if (m_listModel) {
    m_listModel->setRootPath(path);
    m_listView->setRootIndex(m_listModel->index(path));
  }
}

void NMAssetBrowserPanel::onListDoubleClicked(const QModelIndex &index) {
  if (!index.isValid())
    return;

  QString path = m_listModel->filePath(index);
  emit assetDoubleClicked(path);
}

void NMAssetBrowserPanel::onListContextMenu(const QPoint &pos) {
  QModelIndex index = m_listView->indexAt(pos);
  if (!index.isValid())
    return;

  QString path = m_listModel->filePath(index);
  emit assetContextMenu(path, m_listView->mapToGlobal(pos));
}

} // namespace NovelMind::editor::qt
