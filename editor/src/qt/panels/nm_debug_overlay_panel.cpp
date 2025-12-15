#include <NovelMind/editor/qt/nm_icon_manager.hpp>
#include <NovelMind/editor/qt/nm_play_mode_controller.hpp>
#include <NovelMind/editor/qt/panels/nm_debug_overlay_panel.hpp>
#include <QButtonGroup>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QPushButton>
#include <QToolButton>

namespace NovelMind::editor::qt {

NMDebugOverlayPanel::NMDebugOverlayPanel(QWidget *parent)
    : NMDockPanel("Debug Overlay", parent) {
  setupUI();
}

void NMDebugOverlayPanel::onInitialize() {
  NMDockPanel::onInitialize();

  auto &controller = NMPlayModeController::instance();

  // Connect to controller signals
  connect(&controller, &NMPlayModeController::variablesChanged, this,
          &NMDebugOverlayPanel::onVariablesChanged);
  connect(&controller, &NMPlayModeController::callStackChanged, this,
          &NMDebugOverlayPanel::onCallStackChanged);

  // Initial update
  updateVariablesTab(controller.currentVariables());
  updateCallStackTab(controller.callStack());
}

void NMDebugOverlayPanel::onShutdown() { NMDockPanel::onShutdown(); }

void NMDebugOverlayPanel::onUpdate(double deltaTime) {
  NMDockPanel::onUpdate(deltaTime);
}

void NMDebugOverlayPanel::setupUI() {
  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  // Add toolbar
  setupToolBar();
  layout->addWidget(m_toolBar);

  m_tabWidget = new QTabWidget;

  auto &iconMgr = NMIconManager::instance();

  // === Variables Tab ===
  {
    auto *varWidget = new QWidget;
    auto *varLayout = new QVBoxLayout(varWidget);
    varLayout->setContentsMargins(4, 4, 4, 4);

    m_variablesTree = new QTreeWidget;
    m_variablesTree->setHeaderLabels({"Name", "Value", "Type"});
    m_variablesTree->setAlternatingRowColors(true);
    m_variablesTree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_variablesTree->header()->setStretchLastSection(false);
    m_variablesTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_variablesTree->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_variablesTree->header()->setSectionResizeMode(
        2, QHeaderView::ResizeToContents);

    connect(m_variablesTree, &QTreeWidget::itemDoubleClicked, this,
            &NMDebugOverlayPanel::onVariableItemDoubleClicked);

    auto *helpLabel =
        new QLabel("💡 Double-click a variable to edit (only when paused)");
    helpLabel->setStyleSheet(
        "QLabel { color: #a0a0a0; font-size: 9pt; padding: 4px; }");

    varLayout->addWidget(m_variablesTree);
    varLayout->addWidget(helpLabel);

    m_tabWidget->addTab(varWidget, iconMgr.getIcon("info", 16), "Variables");
  }

  // === Call Stack Tab ===
  {
    auto *stackWidget = new QWidget;
    auto *stackLayout = new QVBoxLayout(stackWidget);
    stackLayout->setContentsMargins(4, 4, 4, 4);

    m_callStackList = new QListWidget;
    m_callStackList->setAlternatingRowColors(true);

    stackLayout->addWidget(m_callStackList);

    m_tabWidget->addTab(stackWidget, "Call Stack");
  }

  // === Current Instruction Tab ===
  {
    m_instructionWidget = new QWidget;
    auto *instrLayout = new QGridLayout(m_instructionWidget);
    instrLayout->setContentsMargins(8, 8, 8, 8);
    instrLayout->setSpacing(8);

    // Current Node
    auto *nodeHeaderLabel = new QLabel("<b>Current Node:</b>");
    instrLayout->addWidget(nodeHeaderLabel, 0, 0, Qt::AlignTop);

    m_currentNodeLabel = new QLabel("DialogueNode_003");
    m_currentNodeLabel->setStyleSheet(
        "QLabel { color: #0078d4; font-family: monospace; }");
    instrLayout->addWidget(m_currentNodeLabel, 0, 1);

    // Instruction Index
    auto *indexHeaderLabel = new QLabel("<b>Instruction Index:</b>");
    instrLayout->addWidget(indexHeaderLabel, 1, 0, Qt::AlignTop);

    m_instructionIndexLabel = new QLabel("5 / 12");
    m_instructionIndexLabel->setStyleSheet(
        "QLabel { color: #4caf50; font-family: monospace; }");
    instrLayout->addWidget(m_instructionIndexLabel, 1, 1);

    // Separator
    auto *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    instrLayout->addWidget(separator, 2, 0, 1, 2);

    // Current Instruction Code
    auto *codeHeaderLabel = new QLabel("<b>Current Instruction:</b>");
    instrLayout->addWidget(codeHeaderLabel, 3, 0, Qt::AlignTop);

    m_instructionCodeLabel = new QLabel("SHOW_TEXT \"Hello, world!\"");
    m_instructionCodeLabel->setStyleSheet("QLabel { "
                                          "  background-color: #1e1e1e; "
                                          "  color: #e0e0e0; "
                                          "  font-family: monospace; "
                                          "  padding: 8px; "
                                          "  border: 1px solid #3d3d3d; "
                                          "  border-radius: 3px; "
                                          "}");
    m_instructionCodeLabel->setWordWrap(true);
    instrLayout->addWidget(m_instructionCodeLabel, 3, 1);

    instrLayout->setRowStretch(4, 1); // Push content to top

    m_tabWidget->addTab(m_instructionWidget, "Current Instruction");
  }

  // === Animations Tab ===
  {
    auto *animWidget = new QWidget;
    auto *animLayout = new QVBoxLayout(animWidget);
    animLayout->setContentsMargins(4, 4, 4, 4);

    m_animationsTree = new QTreeWidget;
    m_animationsTree->setHeaderLabels({"Animation", "Progress", "Target"});
    m_animationsTree->setAlternatingRowColors(true);

    // Add placeholder items
    auto *item1 = new QTreeWidgetItem(
        m_animationsTree, {"character_fade", "65%", "Character_Alice"});
    auto *item2 = new QTreeWidgetItem(
        m_animationsTree, {"bg_transition", "95%", "Background_School"});
    item1->setForeground(1, QBrush(QColor("#4caf50")));
    item2->setForeground(1, QBrush(QColor("#4caf50")));

    animLayout->addWidget(m_animationsTree);

    m_tabWidget->addTab(animWidget, "Animations");
  }

  // === Audio Tab ===
  {
    auto *audioWidget = new QWidget;
    auto *audioLayout = new QVBoxLayout(audioWidget);
    audioLayout->setContentsMargins(4, 4, 4, 4);

    m_audioTree = new QTreeWidget;
    m_audioTree->setHeaderLabels({"Channel", "File", "Volume", "State"});
    m_audioTree->setAlternatingRowColors(true);

    // Add placeholder items
    auto *item1 = new QTreeWidgetItem(
        m_audioTree, {"BGM", "music_menu.ogg", "80%", "Playing"});
    auto *item2 = new QTreeWidgetItem(
        m_audioTree, {"SFX_1", "door_open.wav", "100%", "Stopped"});
    item1->setForeground(3, QBrush(QColor("#4caf50")));
    item2->setForeground(3, QBrush(QColor("#a0a0a0")));

    audioLayout->addWidget(m_audioTree);

    m_tabWidget->addTab(audioWidget, "Audio");
  }

  // === Performance Tab ===
  {
    auto *perfWidget = new QWidget;
    auto *perfLayout = new QVBoxLayout(perfWidget);
    perfLayout->setContentsMargins(4, 4, 4, 4);

    m_performanceTree = new QTreeWidget;
    m_performanceTree->setHeaderLabels({"Metric", "Value"});
    m_performanceTree->setAlternatingRowColors(true);
    m_performanceTree->header()->setStretchLastSection(true);

    // Add placeholder metrics
    new QTreeWidgetItem(m_performanceTree, {"Frame Time", "16.7 ms"});
    new QTreeWidgetItem(m_performanceTree, {"FPS", "60"});
    new QTreeWidgetItem(m_performanceTree, {"Memory Usage", "45 MB"});
    new QTreeWidgetItem(m_performanceTree, {"Active Objects", "12"});
    new QTreeWidgetItem(m_performanceTree,
                        {"Script Instructions/sec", "1,250"});

    perfLayout->addWidget(m_performanceTree);

    m_tabWidget->addTab(perfWidget, "Performance");
  }

  layout->addWidget(m_tabWidget);
  setLayout(layout);
}

void NMDebugOverlayPanel::updateVariablesTab(const QVariantMap &variables) {
  m_currentVariables = variables;

  m_variablesTree->clear();

  // Create top-level groups
  auto *globalGroup =
      new QTreeWidgetItem(m_variablesTree, {"📁 Global Variables", "", ""});
  globalGroup->setExpanded(true);
  globalGroup->setForeground(0, QBrush(QColor("#0078d4")));

  auto *localGroup =
      new QTreeWidgetItem(m_variablesTree, {"📁 Local Variables", "", ""});
  localGroup->setExpanded(true);
  localGroup->setForeground(0, QBrush(QColor("#0078d4")));

  // Populate variables (all go to global for now)
  for (auto it = variables.constBegin(); it != variables.constEnd(); ++it) {
    const QString &name = it.key();
    const QVariant &value = it.value();

    QString valueStr = value.toString();
    QString typeStr = value.typeName();

    // Add quotes for strings
    if (value.metaType().id() == QMetaType::QString) {
      valueStr = QString("\"%1\"").arg(valueStr);
    }

    auto *item = new QTreeWidgetItem(globalGroup, {name, valueStr, typeStr});

    // Color-code by type
    QColor valueColor;
    if (value.metaType().id() == QMetaType::QString) {
      valueColor = QColor("#ce9178"); // String color
    } else if (value.metaType().id() == QMetaType::Int ||
               value.metaType().id() == QMetaType::Double) {
      valueColor = QColor("#b5cea8"); // Number color
    } else {
      valueColor = QColor("#e0e0e0"); // Default
    }
    item->setForeground(1, QBrush(valueColor));
    item->setForeground(2, QBrush(QColor("#a0a0a0")));

    // Store variable name in item data for editing
    item->setData(0, Qt::UserRole, name);
  }

  // Add placeholder local variable
  auto *tempItem = new QTreeWidgetItem(
      localGroup, {"tempChoice", "\"Option A\"", "QString"});
  tempItem->setForeground(1, QBrush(QColor("#ce9178")));
  tempItem->setForeground(2, QBrush(QColor("#a0a0a0")));
}

void NMDebugOverlayPanel::updateCallStackTab(const QStringList &stack) {
  m_currentCallStack = stack;

  m_callStackList->clear();

  for (int i = static_cast<int>(stack.size()) - 1; i >= 0;
       --i) { // Reverse order (top of stack first)
    const QString &frame = stack[i];
    auto *item =
        new QListWidgetItem(QString("%1. %2").arg(stack.size() - i).arg(frame));

    if (i == stack.size() - 1) {
      // Highlight current frame
      item->setForeground(QBrush(QColor("#0078d4")));
      item->setIcon(NMIconManager::instance().getIcon("arrow-right", 16));
    }

    m_callStackList->addItem(item);
  }
}

void NMDebugOverlayPanel::onVariablesChanged(const QVariantMap &variables) {
  updateVariablesTab(variables);
}

void NMDebugOverlayPanel::onCallStackChanged(const QStringList &stack) {
  updateCallStackTab(stack);
}

void NMDebugOverlayPanel::onPlayModeChanged([[maybe_unused]] int mode) {
  // Update UI based on play mode
}

void NMDebugOverlayPanel::onVariableItemDoubleClicked(
    QTreeWidgetItem *item, [[maybe_unused]] int column) {
  if (!item->parent()) {
    // Clicked on a group, not a variable
    return;
  }

  auto &controller = NMPlayModeController::instance();

  if (!controller.isPaused()) {
    // Only allow editing when paused
    m_variablesTree->setToolTip(
        "Variables can only be edited when playback is paused");
    return;
  }

  const QString varName = item->data(0, Qt::UserRole).toString();
  if (varName.isEmpty()) {
    return; // Placeholder item
  }

  const QVariant currentValue = m_currentVariables.value(varName);
  editVariable(varName, currentValue);
}

void NMDebugOverlayPanel::editVariable(const QString &name,
                                       const QVariant &currentValue) {
  bool ok = false;
  QVariant newValue;

  if (currentValue.metaType().id() == QMetaType::QString) {
    newValue = QInputDialog::getText(
        this, "Edit Variable", QString("Enter new value for '%1':").arg(name),
        QLineEdit::Normal, currentValue.toString(), &ok);
  } else if (currentValue.metaType().id() == QMetaType::Int) {
    newValue = QInputDialog::getInt(
        this, "Edit Variable", QString("Enter new value for '%1':").arg(name),
        currentValue.toInt(), -2147483647, 2147483647, 1, &ok);
  } else if (currentValue.metaType().id() == QMetaType::Double) {
    newValue = QInputDialog::getDouble(
        this, "Edit Variable", QString("Enter new value for '%1':").arg(name),
        currentValue.toDouble(), -std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max(), 2, &ok);
  } else {
    // Unsupported type, edit as string
    newValue = QInputDialog::getText(
        this, "Edit Variable", QString("Enter new value for '%1':").arg(name),
        QLineEdit::Normal, currentValue.toString(), &ok);
  }

  if (ok) {
    NMPlayModeController::instance().setVariable(name, newValue);
  }
}

void NMDebugOverlayPanel::setupToolBar() {
  m_toolBar = new QToolBar;
  m_toolBar->setObjectName("DebugOverlayToolBar");
  m_toolBar->setIconSize(QSize(16, 16));

  [[maybe_unused]] auto &iconMgr = NMIconManager::instance();

  // Display mode toggle
  auto *minimalBtn = new QToolButton;
  minimalBtn->setText("Minimal");
  minimalBtn->setCheckable(true);
  minimalBtn->setToolTip("Show only essential debugging info");

  auto *extendedBtn = new QToolButton;
  extendedBtn->setText("Extended");
  extendedBtn->setCheckable(true);
  extendedBtn->setChecked(true);
  extendedBtn->setToolTip("Show all debugging information");

  auto *modeGroup = new QButtonGroup(this);
  modeGroup->addButton(minimalBtn, static_cast<int>(DebugDisplayMode::Minimal));
  modeGroup->addButton(extendedBtn,
                       static_cast<int>(DebugDisplayMode::Extended));

  connect(modeGroup, QOverload<int>::of(&QButtonGroup::idClicked), this,
          &NMDebugOverlayPanel::onDisplayModeChanged);

  m_toolBar->addWidget(new QLabel("Display Mode: "));
  m_toolBar->addWidget(minimalBtn);
  m_toolBar->addWidget(extendedBtn);
}

void NMDebugOverlayPanel::setDisplayMode(DebugDisplayMode mode) {
  if (m_displayMode == mode)
    return;

  m_displayMode = mode;
  updateTabsVisibility();
}

void NMDebugOverlayPanel::updateTabsVisibility() {
  if (!m_tabWidget)
    return;

  switch (m_displayMode) {
  case DebugDisplayMode::Minimal:
    // Show only Variables and Current Instruction
    m_tabWidget->setTabVisible(0, true);  // Variables
    m_tabWidget->setTabVisible(1, false); // Call Stack
    m_tabWidget->setTabVisible(2, true);  // Current Instruction
    m_tabWidget->setTabVisible(3, false); // Animations
    m_tabWidget->setTabVisible(4, false); // Audio
    m_tabWidget->setTabVisible(5, false); // Performance
    break;

  case DebugDisplayMode::Extended:
    // Show all tabs
    for (int i = 0; i < m_tabWidget->count(); ++i) {
      m_tabWidget->setTabVisible(i, true);
    }
    break;
  }
}

void NMDebugOverlayPanel::onDisplayModeChanged() {
  auto *sender = qobject_cast<QButtonGroup *>(this->sender());
  if (!sender)
    return;

  int modeId = sender->checkedId();
  setDisplayMode(static_cast<DebugDisplayMode>(modeId));
}

void NMDebugOverlayPanel::updateCurrentInstructionTab() {
  // This would be updated from the PlayModeController
  // For now, using mock data
  auto &controller = NMPlayModeController::instance();

  if (controller.isPlaying() || controller.isPaused()) {
    m_currentNodeLabel->setText("DialogueNode_003");
    m_instructionIndexLabel->setText("5 / 12");
    m_instructionCodeLabel->setText("SHOW_TEXT \"Hello, world!\"");
  } else {
    m_currentNodeLabel->setText("(Not running)");
    m_instructionIndexLabel->setText("- / -");
    m_instructionCodeLabel->setText("(No active instruction)");
  }
}

} // namespace NovelMind::editor::qt
