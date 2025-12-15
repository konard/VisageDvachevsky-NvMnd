#include "NovelMind/editor/qt/panels/nm_localization_panel.hpp"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QToolBar>
#include <QVBoxLayout>

namespace NovelMind::editor::qt {

NMLocalizationPanel::NMLocalizationPanel(QWidget *parent)
    : NMDockPanel("Localization Manager", parent) {}

NMLocalizationPanel::~NMLocalizationPanel() = default;

void NMLocalizationPanel::onInitialize() { setupUI(); }

void NMLocalizationPanel::onShutdown() {}

void NMLocalizationPanel::setupUI() {
  QVBoxLayout *layout = new QVBoxLayout(contentWidget());
  layout->setContentsMargins(4, 4, 4, 4);

  QHBoxLayout *topLayout = new QHBoxLayout();
  topLayout->addWidget(new QLabel("Language:", contentWidget()));
  m_languageSelector = new QComboBox(contentWidget());
  m_languageSelector->addItems(
      {"English", "Japanese", "Spanish", "French", "German", "Russian"});
  topLayout->addWidget(m_languageSelector);
  topLayout->addStretch();
  topLayout->addWidget(new QPushButton("Import", contentWidget()));
  topLayout->addWidget(new QPushButton("Export", contentWidget()));
  layout->addLayout(topLayout);

  m_stringsTable = new QTableWidget(contentWidget());
  m_stringsTable->setColumnCount(3);
  m_stringsTable->setHorizontalHeaderLabels({"ID", "Source", "Translation"});
  m_stringsTable->setRowCount(3);
  m_stringsTable->setItem(0, 0, new QTableWidgetItem("menu.start"));
  m_stringsTable->setItem(0, 1, new QTableWidgetItem("Start Game"));
  m_stringsTable->setItem(0, 2, new QTableWidgetItem(""));
  m_stringsTable->setItem(1, 0, new QTableWidgetItem("menu.load"));
  m_stringsTable->setItem(1, 1, new QTableWidgetItem("Load Game"));
  m_stringsTable->setItem(1, 2, new QTableWidgetItem(""));
  m_stringsTable->setItem(2, 0, new QTableWidgetItem("dialogue.hello"));
  m_stringsTable->setItem(2, 1, new QTableWidgetItem("Hello, welcome!"));
  m_stringsTable->setItem(2, 2, new QTableWidgetItem(""));
  layout->addWidget(m_stringsTable, 1);
}

void NMLocalizationPanel::onUpdate([[maybe_unused]] double deltaTime) {}

} // namespace NovelMind::editor::qt
