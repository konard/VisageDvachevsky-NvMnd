#include "NovelMind/editor/qt/panels/nm_voice_manager_panel.hpp"

#include <QListWidget>
#include <QPushButton>
#include <QToolBar>
#include <QVBoxLayout>

namespace NovelMind::editor::qt {

NMVoiceManagerPanel::NMVoiceManagerPanel(QWidget *parent)
    : NMDockPanel("Voice Manager", parent) {}

NMVoiceManagerPanel::~NMVoiceManagerPanel() = default;

void NMVoiceManagerPanel::onInitialize() { setupUI(); }

void NMVoiceManagerPanel::onShutdown() {}

void NMVoiceManagerPanel::setupUI() {
  QVBoxLayout *layout = new QVBoxLayout(contentWidget());
  layout->setContentsMargins(0, 0, 0, 0);

  m_toolbar = new QToolBar(contentWidget());
  m_toolbar->addWidget(new QPushButton("Import Voice", m_toolbar));
  m_toolbar->addWidget(new QPushButton("Play", m_toolbar));
  m_toolbar->addWidget(new QPushButton("Stop", m_toolbar));
  layout->addWidget(m_toolbar);

  m_voiceList = new QListWidget(contentWidget());
  m_voiceList->addItems(
      {"character1_line1.wav", "character2_line1.wav", "narrator_intro.wav"});
  layout->addWidget(m_voiceList, 1);
}

void NMVoiceManagerPanel::onUpdate([[maybe_unused]] double deltaTime) {}

} // namespace NovelMind::editor::qt
