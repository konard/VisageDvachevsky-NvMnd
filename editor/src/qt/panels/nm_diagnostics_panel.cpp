#include "NovelMind/editor/qt/panels/nm_diagnostics_panel.hpp"

#include <QPushButton>
#include <QToolBar>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace NovelMind::editor::qt {

NMDiagnosticsPanel::NMDiagnosticsPanel(QWidget *parent)
    : NMDockPanel("Diagnostics", parent) {}

NMDiagnosticsPanel::~NMDiagnosticsPanel() = default;

void NMDiagnosticsPanel::onInitialize() { setupUI(); }

void NMDiagnosticsPanel::onShutdown() {}

void NMDiagnosticsPanel::setupUI() {
  QVBoxLayout *layout = new QVBoxLayout(contentWidget());
  layout->setContentsMargins(0, 0, 0, 0);

  m_toolbar = new QToolBar(contentWidget());
  m_toolbar->addWidget(new QPushButton("Clear All", m_toolbar));
  m_toolbar->addSeparator();
  m_toolbar->addWidget(new QPushButton("Errors", m_toolbar));
  m_toolbar->addWidget(new QPushButton("Warnings", m_toolbar));
  m_toolbar->addWidget(new QPushButton("Info", m_toolbar));
  layout->addWidget(m_toolbar);

  m_diagnosticsTree = new QTreeWidget(contentWidget());
  m_diagnosticsTree->setHeaderLabels({"Type", "Message", "File", "Line"});

  // Add example diagnostics
  QTreeWidgetItem *error = new QTreeWidgetItem(m_diagnosticsTree);
  error->setText(0, "Error");
  error->setText(1, "Undefined variable 'player_name'");
  error->setText(2, "scene1.nm");
  error->setText(3, "42");
  error->setForeground(0, QBrush(QColor("#f44336")));

  QTreeWidgetItem *warning = new QTreeWidgetItem(m_diagnosticsTree);
  warning->setText(0, "Warning");
  warning->setText(1, "Unused audio file 'background2.wav'");
  warning->setText(2, "assets/audio/");
  warning->setText(3, "-");
  warning->setForeground(0, QBrush(QColor("#ff9800")));

  layout->addWidget(m_diagnosticsTree, 1);
}

void NMDiagnosticsPanel::onUpdate([[maybe_unused]] double deltaTime) {}

} // namespace NovelMind::editor::qt
