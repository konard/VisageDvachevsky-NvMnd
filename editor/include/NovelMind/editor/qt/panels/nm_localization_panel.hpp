#pragma once

/**
 * @file nm_localization_panel.hpp
 * @brief Localization and translation management
 */

#include "NovelMind/editor/qt/nm_dock_panel.hpp"

#include <QWidget>

class QTableWidget;
class QToolBar;
class QComboBox;

namespace NovelMind::editor::qt {

class NMLocalizationPanel : public NMDockPanel {
  Q_OBJECT

public:
  explicit NMLocalizationPanel(QWidget *parent = nullptr);
  ~NMLocalizationPanel() override;

  void onInitialize() override;
  void onShutdown() override;
  void onUpdate(double deltaTime) override;

private:
  void setupUI();

  QTableWidget *m_stringsTable = nullptr;
  QComboBox *m_languageSelector = nullptr;
};

} // namespace NovelMind::editor::qt
