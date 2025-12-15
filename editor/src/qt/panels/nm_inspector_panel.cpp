#include "NovelMind/editor/qt/panels/nm_inspector_panel.hpp"
#include "NovelMind/editor/qt/nm_style_manager.hpp"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace NovelMind::editor::qt {

// ============================================================================
// NMPropertyGroup
// ============================================================================

NMPropertyGroup::NMPropertyGroup(const QString &title, QWidget *parent)
    : QWidget(parent) {
  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // Header
  m_header = new QWidget(this);
  m_header->setCursor(Qt::PointingHandCursor);
  auto *headerLayout = new QHBoxLayout(m_header);
  headerLayout->setContentsMargins(4, 4, 4, 4);

  m_expandIcon =
      new QLabel(QString::fromUtf8("\u25BC"), m_header); // Down arrow
  m_expandIcon->setFixedWidth(16);
  headerLayout->addWidget(m_expandIcon);

  auto *titleLabel = new QLabel(title, m_header);
  QFont boldFont = titleLabel->font();
  boldFont.setBold(true);
  titleLabel->setFont(boldFont);
  headerLayout->addWidget(titleLabel);

  headerLayout->addStretch();

  // Make header clickable
  m_header->installEventFilter(this);

  mainLayout->addWidget(m_header);

  // Separator
  auto *separator = new QFrame(this);
  separator->setFrameShape(QFrame::HLine);
  separator->setFrameShadow(QFrame::Sunken);
  mainLayout->addWidget(separator);

  // Content area
  m_content = new QWidget(this);
  m_contentLayout = new QVBoxLayout(m_content);
  m_contentLayout->setContentsMargins(8, 4, 8, 8);
  m_contentLayout->setSpacing(4);

  mainLayout->addWidget(m_content);

  // Connect header click
  connect(m_header, &QWidget::destroyed, [] {}); // Placeholder for event filter
}

void NMPropertyGroup::setExpanded(bool expanded) {
  m_expanded = expanded;
  m_content->setVisible(expanded);
  m_expandIcon->setText(expanded ? QString::fromUtf8("\u25BC")
                                 : QString::fromUtf8("\u25B6"));
}

void NMPropertyGroup::addProperty(const QString &name, const QString &value) {
  auto *row = new QWidget(m_content);
  auto *rowLayout = new QHBoxLayout(row);
  rowLayout->setContentsMargins(0, 0, 0, 0);
  rowLayout->setSpacing(8);

  auto *nameLabel = new QLabel(name + ":", row);
  nameLabel->setMinimumWidth(100);
  nameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  const auto &palette = NMStyleManager::instance().palette();
  nameLabel->setStyleSheet(
      QString("color: %1;")
          .arg(NMStyleManager::colorToStyleString(palette.textSecondary)));

  auto *valueLabel = new QLabel(value, row);
  valueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

  rowLayout->addWidget(nameLabel);
  rowLayout->addWidget(valueLabel, 1);

  m_contentLayout->addWidget(row);
}

void NMPropertyGroup::addProperty(const QString &name, QWidget *widget) {
  auto *row = new QWidget(m_content);
  auto *rowLayout = new QHBoxLayout(row);
  rowLayout->setContentsMargins(0, 0, 0, 0);
  rowLayout->setSpacing(8);

  auto *nameLabel = new QLabel(name + ":", row);
  nameLabel->setMinimumWidth(100);
  nameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  const auto &palette = NMStyleManager::instance().palette();
  nameLabel->setStyleSheet(
      QString("color: %1;")
          .arg(NMStyleManager::colorToStyleString(palette.textSecondary)));

  rowLayout->addWidget(nameLabel);
  rowLayout->addWidget(widget, 1);

  m_contentLayout->addWidget(row);
}

void NMPropertyGroup::clearProperties() {
  QLayoutItem *item;
  while ((item = m_contentLayout->takeAt(0)) != nullptr) {
    delete item->widget();
    delete item;
  }
}

void NMPropertyGroup::onHeaderClicked() { setExpanded(!m_expanded); }

void NMPropertyGroup::addEditableProperty(const QString &name,
                                          NMPropertyType type,
                                          const QString &currentValue,
                                          const QStringList &enumValues) {
  QWidget *editor = nullptr;
  const auto &palette = NMStyleManager::instance().palette();

  switch (type) {
  case NMPropertyType::String: {
    auto *lineEdit = new QLineEdit(currentValue);
    lineEdit->setProperty("propertyName", name);
    lineEdit->setStyleSheet(QString("QLineEdit {"
                                    "  background-color: %1;"
                                    "  color: %2;"
                                    "  border: 1px solid %3;"
                                    "  border-radius: 3px;"
                                    "  padding: 4px;"
                                    "}"
                                    "QLineEdit:focus {"
                                    "  border-color: %4;"
                                    "}")
                                .arg(palette.bgDark.name())
                                .arg(palette.textPrimary.name())
                                .arg(palette.borderDark.name())
                                .arg(palette.accentPrimary.name()));

    connect(lineEdit, &QLineEdit::editingFinished, this,
            &NMPropertyGroup::onPropertyEdited);
    editor = lineEdit;
    break;
  }

  case NMPropertyType::Integer: {
    auto *spinBox = new QSpinBox();
    spinBox->setProperty("propertyName", name);
    spinBox->setRange(-999999, 999999);
    spinBox->setValue(currentValue.toInt());
    spinBox->setStyleSheet(QString("QSpinBox {"
                                   "  background-color: %1;"
                                   "  color: %2;"
                                   "  border: 1px solid %3;"
                                   "  border-radius: 3px;"
                                   "  padding: 4px;"
                                   "}")
                               .arg(palette.bgDark.name())
                               .arg(palette.textPrimary.name())
                               .arg(palette.borderDark.name()));

    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &NMPropertyGroup::onPropertyEdited);
    editor = spinBox;
    break;
  }

  case NMPropertyType::Float: {
    auto *doubleSpinBox = new QDoubleSpinBox();
    doubleSpinBox->setProperty("propertyName", name);
    doubleSpinBox->setRange(-999999.0, 999999.0);
    doubleSpinBox->setDecimals(3);
    doubleSpinBox->setValue(currentValue.toDouble());
    doubleSpinBox->setStyleSheet(QString("QDoubleSpinBox {"
                                         "  background-color: %1;"
                                         "  color: %2;"
                                         "  border: 1px solid %3;"
                                         "  border-radius: 3px;"
                                         "  padding: 4px;"
                                         "}")
                                     .arg(palette.bgDark.name())
                                     .arg(palette.textPrimary.name())
                                     .arg(palette.borderDark.name()));

    connect(doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &NMPropertyGroup::onPropertyEdited);
    editor = doubleSpinBox;
    break;
  }

  case NMPropertyType::Boolean: {
    auto *checkBox = new QCheckBox();
    checkBox->setProperty("propertyName", name);
    checkBox->setChecked(currentValue.toLower() == "true" ||
                         currentValue == "1");
    checkBox->setStyleSheet(
        QString("QCheckBox { color: %1; }").arg(palette.textPrimary.name()));

    connect(checkBox, &QCheckBox::toggled, this,
            &NMPropertyGroup::onPropertyEdited);
    editor = checkBox;
    break;
  }

  case NMPropertyType::Enum: {
    auto *comboBox = new QComboBox();
    comboBox->setProperty("propertyName", name);
    comboBox->addItems(enumValues);
    comboBox->setCurrentText(currentValue);
    comboBox->setStyleSheet(QString("QComboBox {"
                                    "  background-color: %1;"
                                    "  color: %2;"
                                    "  border: 1px solid %3;"
                                    "  border-radius: 3px;"
                                    "  padding: 4px;"
                                    "}"
                                    "QComboBox::drop-down {"
                                    "  border: none;"
                                    "}"
                                    "QComboBox::down-arrow {"
                                    "  image: none;"
                                    "  border: none;"
                                    "}")
                                .arg(palette.bgDark.name())
                                .arg(palette.textPrimary.name())
                                .arg(palette.borderDark.name()));

    connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &NMPropertyGroup::onPropertyEdited);
    editor = comboBox;
    break;
  }

  case NMPropertyType::Color: {
    auto *colorButton = new QPushButton();
    colorButton->setProperty("propertyName", name);
    colorButton->setFixedHeight(30);

    QColor color(currentValue);
    if (!color.isValid()) {
      color = Qt::white;
    }
    colorButton->setProperty("currentColor", color);

    colorButton->setStyleSheet(QString("QPushButton {"
                                       "  background-color: %1;"
                                       "  border: 1px solid %2;"
                                       "  border-radius: 3px;"
                                       "}"
                                       "QPushButton:hover {"
                                       "  border-color: %3;"
                                       "}")
                                   .arg(color.name())
                                   .arg(palette.borderDark.name())
                                   .arg(palette.accentPrimary.name()));

    connect(colorButton, &QPushButton::clicked, this,
            [this, colorButton, name]() {
              QColor current =
                  colorButton->property("currentColor").value<QColor>();
              QColor newColor = QColorDialog::getColor(
                  current, this, QString("Choose %1").arg(name));

              if (newColor.isValid()) {
                colorButton->setProperty("currentColor", newColor);
                colorButton->setStyleSheet(QString("QPushButton {"
                                                   "  background-color: %1;"
                                                   "  border: 1px solid %2;"
                                                   "  border-radius: 3px;"
                                                   "}")
                                               .arg(newColor.name())
                                               .arg(NMStyleManager::instance()
                                                        .palette()
                                                        .borderDark.name()));

                emit propertyValueChanged(name, newColor.name());
              }
            });

    editor = colorButton;
    break;
  }

  case NMPropertyType::Asset: {
    // For assets, create a button that would open an asset picker dialog
    auto *assetButton = new QPushButton(
        currentValue.isEmpty() ? "(Select Asset)" : currentValue);
    assetButton->setProperty("propertyName", name);
    assetButton->setStyleSheet(QString("QPushButton {"
                                       "  background-color: %1;"
                                       "  color: %2;"
                                       "  border: 1px solid %3;"
                                       "  border-radius: 3px;"
                                       "  padding: 4px;"
                                       "  text-align: left;"
                                       "}"
                                       "QPushButton:hover {"
                                       "  border-color: %4;"
                                       "}")
                                   .arg(palette.bgDark.name())
                                   .arg(palette.textPrimary.name())
                                   .arg(palette.borderDark.name())
                                   .arg(palette.accentPrimary.name()));

    connect(assetButton, &QPushButton::clicked, this, [this, name]() {
      // TODO: Open asset picker dialog
      // For now, just emit a change with placeholder
      emit propertyValueChanged(name, "asset_placeholder.png");
    });

    editor = assetButton;
    break;
  }
  }

  if (editor) {
    addProperty(name, editor);
  }
}

void NMPropertyGroup::onPropertyEdited() {
  QObject *sender = QObject::sender();
  if (!sender)
    return;

  QString propertyName = sender->property("propertyName").toString();
  QString newValue;

  if (auto *lineEdit = qobject_cast<QLineEdit *>(sender)) {
    newValue = lineEdit->text();
  } else if (auto *spinBox = qobject_cast<QSpinBox *>(sender)) {
    newValue = QString::number(spinBox->value());
  } else if (auto *doubleSpinBox = qobject_cast<QDoubleSpinBox *>(sender)) {
    newValue = QString::number(doubleSpinBox->value(), 'f', 3);
  } else if (auto *checkBox = qobject_cast<QCheckBox *>(sender)) {
    newValue = checkBox->isChecked() ? "true" : "false";
  } else if (auto *comboBox = qobject_cast<QComboBox *>(sender)) {
    newValue = comboBox->currentText();
  }

  if (!propertyName.isEmpty()) {
    emit propertyValueChanged(propertyName, newValue);
  }
}

// ============================================================================
// NMInspectorPanel
// ============================================================================

NMInspectorPanel::NMInspectorPanel(QWidget *parent)
    : NMDockPanel(tr("Inspector"), parent) {
  setPanelId("Inspector");
  setupContent();
}

NMInspectorPanel::~NMInspectorPanel() = default;

void NMInspectorPanel::onInitialize() { showNoSelection(); }

void NMInspectorPanel::onUpdate(double /*deltaTime*/) {
  // No continuous update needed
}

void NMInspectorPanel::clear() {
  // Remove all groups
  for (auto *group : m_groups) {
    m_mainLayout->removeWidget(group);
    delete group;
  }
  m_groups.clear();

  m_headerLabel->clear();
}

void NMInspectorPanel::inspectObject(const QString &objectType,
                                     const QString &objectId, bool editable) {
  clear();
  m_noSelectionLabel->hide();
  m_currentObjectId = objectId;
  m_editMode = editable;

  // Set header
  m_headerLabel->setText(
      QString("<b>%1</b><br><span style='color: gray;'>%2</span>")
          .arg(objectType)
          .arg(objectId));
  m_headerLabel->show();

  // Add demo properties based on type
  auto *transformGroup = addGroup(tr("Transform"));

  if (m_editMode) {
    transformGroup->addEditableProperty(tr("Position X"), NMPropertyType::Float,
                                        "0.0");
    transformGroup->addEditableProperty(tr("Position Y"), NMPropertyType::Float,
                                        "0.0");
    transformGroup->addEditableProperty(tr("Rotation"), NMPropertyType::Float,
                                        "0.0");
    transformGroup->addEditableProperty(tr("Scale X"), NMPropertyType::Float,
                                        "1.0");
    transformGroup->addEditableProperty(tr("Scale Y"), NMPropertyType::Float,
                                        "1.0");
  } else {
    transformGroup->addProperty(tr("Position X"), "0.0");
    transformGroup->addProperty(tr("Position Y"), "0.0");
    transformGroup->addProperty(tr("Rotation"), "0.0");
    transformGroup->addProperty(tr("Scale X"), "1.0");
    transformGroup->addProperty(tr("Scale Y"), "1.0");
  }

  connect(transformGroup, &NMPropertyGroup::propertyValueChanged, this,
          &NMInspectorPanel::onGroupPropertyChanged);

  auto *renderGroup = addGroup(tr("Rendering"));

  if (m_editMode) {
    renderGroup->addEditableProperty(tr("Visible"), NMPropertyType::Boolean,
                                     "true");
    renderGroup->addEditableProperty(tr("Alpha"), NMPropertyType::Float, "1.0");
    renderGroup->addEditableProperty(tr("Z-Order"), NMPropertyType::Integer,
                                     "0");
    renderGroup->addEditableProperty(
        tr("Blend Mode"), NMPropertyType::Enum, "Normal",
        {"Normal", "Additive", "Multiply", "Screen", "Overlay"});
    renderGroup->addEditableProperty(tr("Tint Color"), NMPropertyType::Color,
                                     "#FFFFFF");
  } else {
    renderGroup->addProperty(tr("Visible"), "true");
    renderGroup->addProperty(tr("Alpha"), "1.0");
    renderGroup->addProperty(tr("Z-Order"), "0");
  }

  connect(renderGroup, &NMPropertyGroup::propertyValueChanged, this,
          &NMInspectorPanel::onGroupPropertyChanged);

  if (objectType == "Dialogue" || objectType == "Choice") {
    auto *dialogueGroup = addGroup(tr("Dialogue"));

    if (m_editMode) {
      dialogueGroup->addEditableProperty(tr("Speaker"), NMPropertyType::String,
                                         "Narrator");
      dialogueGroup->addEditableProperty(tr("Text"), NMPropertyType::String,
                                         objectId);
      dialogueGroup->addEditableProperty(tr("Voice Clip"),
                                         NMPropertyType::Asset, "");
    } else {
      dialogueGroup->addProperty(tr("Speaker"), "Narrator");
      dialogueGroup->addProperty(tr("Text"), objectId);
      dialogueGroup->addProperty(tr("Voice Clip"), "(none)");
    }

    connect(dialogueGroup, &NMPropertyGroup::propertyValueChanged, this,
            &NMInspectorPanel::onGroupPropertyChanged);
  }

  // Add spacer at the end
  m_mainLayout->addStretch();
}

void NMInspectorPanel::onGroupPropertyChanged(const QString &propertyName,
                                              const QString &newValue) {
  // Emit signal that property was changed
  emit propertyChanged(m_currentObjectId, propertyName, newValue);

  // TODO: Integrate with Undo/Redo system
  // auto* undoManager = NMUndoManager::instance();
  // undoManager->push(new PropertyChangeCommand(m_currentObjectId,
  // propertyName, oldValue, newValue));
}

NMPropertyGroup *NMInspectorPanel::addGroup(const QString &title) {
  auto *group = new NMPropertyGroup(title, m_scrollContent);
  m_mainLayout->addWidget(group);
  m_groups.append(group);
  return group;
}

void NMInspectorPanel::showNoSelection() {
  clear();
  m_headerLabel->hide();
  m_noSelectionLabel->show();
}

void NMInspectorPanel::setupContent() {
  auto *container = new QWidget(this);
  auto *containerLayout = new QVBoxLayout(container);
  containerLayout->setContentsMargins(0, 0, 0, 0);
  containerLayout->setSpacing(0);

  // Header
  m_headerLabel = new QLabel(container);
  m_headerLabel->setWordWrap(true);
  m_headerLabel->setTextFormat(Qt::RichText);
  m_headerLabel->setMargin(8);
  m_headerLabel->hide();
  containerLayout->addWidget(m_headerLabel);

  // Scroll area
  m_scrollArea = new QScrollArea(container);
  m_scrollArea->setWidgetResizable(true);
  m_scrollArea->setFrameShape(QFrame::NoFrame);

  m_scrollContent = new QWidget(m_scrollArea);
  m_mainLayout = new QVBoxLayout(m_scrollContent);
  m_mainLayout->setContentsMargins(0, 0, 0, 0);
  m_mainLayout->setSpacing(8);
  m_mainLayout->setAlignment(Qt::AlignTop);

  m_scrollArea->setWidget(m_scrollContent);
  containerLayout->addWidget(m_scrollArea, 1);

  // No selection label
  m_noSelectionLabel =
      new QLabel(tr("Select an object to view its properties"), container);
  m_noSelectionLabel->setAlignment(Qt::AlignCenter);
  m_noSelectionLabel->setWordWrap(true);

  const auto &palette = NMStyleManager::instance().palette();
  m_noSelectionLabel->setStyleSheet(
      QString("color: %1; padding: 20px;")
          .arg(NMStyleManager::colorToStyleString(palette.textSecondary)));

  m_mainLayout->addWidget(m_noSelectionLabel);

  setContentWidget(container);
}

} // namespace NovelMind::editor::qt
