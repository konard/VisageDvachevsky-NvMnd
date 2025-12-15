#pragma once

/**
 * @file nm_voice_manager_panel.hpp
 * @brief Voice file management panel
 */

#include "NovelMind/editor/qt/nm_dock_panel.hpp"

#include <QToolBar>
#include <QWidget>

class QListWidget;
class QToolBar;
class QPushButton;
class QLabel;

namespace NovelMind::editor::qt {

class NMVoiceManagerPanel : public NMDockPanel {
  Q_OBJECT

public:
  explicit NMVoiceManagerPanel(QWidget *parent = nullptr);
  ~NMVoiceManagerPanel() override;

  void onInitialize() override;
  void onShutdown() override;
  void onUpdate(double deltaTime) override;

private:
  void setupUI();

  QListWidget *m_voiceList = nullptr;
  QToolBar *m_toolbar = nullptr;
};

} // namespace NovelMind::editor::qt
