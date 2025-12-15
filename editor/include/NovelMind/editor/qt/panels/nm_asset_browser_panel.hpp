#pragma once

/**
 * @file nm_asset_browser_panel.hpp
 * @brief Asset Browser panel for managing project assets
 *
 * Provides:
 * - Directory tree navigation
 * - Asset grid/list view
 * - Asset preview
 * - Import/export controls
 */

#include "NovelMind/editor/qt/nm_dock_panel.hpp"
#include <QFileSystemModel>
#include <QListView>
#include <QSplitter>
#include <QToolBar>
#include <QTreeView>

namespace NovelMind::editor::qt {

/**
 * @brief Asset Browser panel for asset management
 */
class NMAssetBrowserPanel : public NMDockPanel {
  Q_OBJECT

public:
  explicit NMAssetBrowserPanel(QWidget *parent = nullptr);
  ~NMAssetBrowserPanel() override;

  void onInitialize() override;
  void onUpdate(double deltaTime) override;

  /**
   * @brief Set the root path for the asset browser
   */
  void setRootPath(const QString &path);

  /**
   * @brief Get the currently selected asset path
   */
  [[nodiscard]] QString selectedAssetPath() const;

  /**
   * @brief Refresh the asset view
   */
  void refresh();

signals:
  void assetSelected(const QString &path);
  void assetDoubleClicked(const QString &path);
  void assetContextMenu(const QString &path, const QPoint &globalPos);

private slots:
  void onTreeClicked(const QModelIndex &index);
  void onListDoubleClicked(const QModelIndex &index);
  void onListContextMenu(const QPoint &pos);

private:
  void setupToolBar();
  void setupContent();

  QSplitter *m_splitter = nullptr;
  QTreeView *m_treeView = nullptr;
  QListView *m_listView = nullptr;
  QFileSystemModel *m_treeModel = nullptr;
  QFileSystemModel *m_listModel = nullptr;
  QWidget *m_contentWidget = nullptr;
  QToolBar *m_toolBar = nullptr;

  QString m_rootPath;
  QString m_currentPath;
};

} // namespace NovelMind::editor::qt
