#pragma once

/**
 * @file nm_build_settings_panel.hpp
 * @brief Build and export settings panel
 */

#include "NovelMind/editor/qt/nm_dock_panel.hpp"

#include <QWidget>

class QFormLayout;
class QComboBox;
class QLineEdit;
class QCheckBox;
class QPushButton;

namespace NovelMind::editor::qt {

class NMBuildSettingsPanel : public NMDockPanel {
  Q_OBJECT

public:
  explicit NMBuildSettingsPanel(QWidget *parent = nullptr);
  ~NMBuildSettingsPanel() override;

  void onInitialize() override;
  void onShutdown() override;
  void onUpdate(double deltaTime) override;

private:
  void setupUI();

  QComboBox *m_platformSelector = nullptr;
  QLineEdit *m_outputPathEdit = nullptr;
  QPushButton *m_buildButton = nullptr;
};

} // namespace NovelMind::editor::qt
