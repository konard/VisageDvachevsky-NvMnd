#include "NovelMind/editor/qt/nm_style_manager.hpp"

#include <QFontDatabase>
#include <QGuiApplication>
#include <QScreen>
#include <QStyleFactory>

namespace NovelMind::editor::qt {

NMStyleManager &NMStyleManager::instance() {
  static NMStyleManager instance;
  return instance;
}

NMStyleManager::NMStyleManager() : QObject(nullptr) {}

void NMStyleManager::initialize(QApplication *app) {
  m_app = app;

  setupHighDpi();
  setupFonts();
  applyDarkTheme();
}

void NMStyleManager::setupHighDpi() {
  // Get the primary screen's DPI
  if (QGuiApplication::primaryScreen()) {
    double dpi = QGuiApplication::primaryScreen()->logicalDotsPerInch();
    // Standard DPI is 96, calculate scale factor
    m_uiScale = dpi / 96.0;

    // Clamp to reasonable range
    if (m_uiScale < 1.0)
      m_uiScale = 1.0;
    if (m_uiScale > 3.0)
      m_uiScale = 3.0;
  }

  // Update icon sizes based on scale
  m_toolbarIconSize = static_cast<int>(24 * m_uiScale);
  m_menuIconSize = static_cast<int>(16 * m_uiScale);
}

void NMStyleManager::setupFonts() {
  // Default UI font
#ifdef Q_OS_WIN
  m_defaultFont = QFont("Segoe UI", 9);
  m_monospaceFont = QFont("Consolas", 9);
#elif defined(Q_OS_LINUX)
  m_defaultFont = QFont("Ubuntu", 10);
  m_monospaceFont = QFont("Ubuntu Mono", 10);

  // Fallback if Ubuntu font not available
  if (!QFontDatabase::families().contains("Ubuntu")) {
    m_defaultFont = QFont("Sans", 10);
  }
  if (!QFontDatabase::families().contains("Ubuntu Mono")) {
    m_monospaceFont = QFont("Monospace", 10);
  }
#else
  m_defaultFont = QFont(); // System default
  m_defaultFont.setPointSize(10);
  m_monospaceFont = QFont("Courier", 10);
#endif

  // Apply scale to fonts
  m_defaultFont.setPointSizeF(m_defaultFont.pointSizeF() * m_uiScale);
  m_monospaceFont.setPointSizeF(m_monospaceFont.pointSizeF() * m_uiScale);
}

void NMStyleManager::applyDarkTheme() {
  if (!m_app)
    return;

  // Use Fusion style as base (cross-platform, customizable)
  m_app->setStyle(QStyleFactory::create("Fusion"));

  // Apply default font
  m_app->setFont(m_defaultFont);

  // Apply stylesheet
  m_app->setStyleSheet(getStyleSheet());

  emit themeChanged();
}

void NMStyleManager::setUiScale(double scale) {
  if (scale < 0.5)
    scale = 0.5;
  if (scale > 3.0)
    scale = 3.0;

  if (qFuzzyCompare(m_uiScale, scale))
    return;

  m_uiScale = scale;
  m_toolbarIconSize = static_cast<int>(24 * m_uiScale);
  m_menuIconSize = static_cast<int>(16 * m_uiScale);

  setupFonts();
  applyDarkTheme();

  emit scaleChanged(m_uiScale);
}

QString NMStyleManager::colorToStyleString(const QColor &color) {
  return QString("rgb(%1, %2, %3)")
      .arg(color.red())
      .arg(color.green())
      .arg(color.blue());
}

QString NMStyleManager::colorToRgbaString(const QColor &color, int alpha) {
  return QString("rgba(%1, %2, %3, %4)")
      .arg(color.red())
      .arg(color.green())
      .arg(color.blue())
      .arg(alpha);
}

QString NMStyleManager::getStyleSheet() const {
  const auto &p = m_palette;

  // Generate comprehensive stylesheet for Unreal-like dark theme
  return QString(R"(
/* ========================================================================== */
/* Global Styles                                                              */
/* ========================================================================== */

* {
    color: %1;
    background-color: %2;
    selection-background-color: %3;
    selection-color: %4;
}

/* ========================================================================== */
/* QMainWindow                                                                */
/* ========================================================================== */

QMainWindow {
    background-color: %5;
}

QMainWindow::separator {
    background-color: %6;
    width: 2px;
    height: 2px;
}

QMainWindow::separator:hover {
    background-color: %3;
}

/* ========================================================================== */
/* QMenuBar                                                                   */
/* ========================================================================== */

QMenuBar {
    background-color: %2;
    border-bottom: 1px solid %6;
    padding: 2px;
}

QMenuBar::item {
    background-color: transparent;
    padding: 4px 8px;
    border-radius: 2px;
}

QMenuBar::item:selected {
    background-color: %7;
}

QMenuBar::item:pressed {
    background-color: %3;
}

/* ========================================================================== */
/* QMenu                                                                      */
/* ========================================================================== */

QMenu {
    background-color: %2;
    border: 1px solid %6;
    padding: 4px;
}

QMenu::item {
    padding: 6px 24px 6px 8px;
    border-radius: 2px;
}

QMenu::item:selected {
    background-color: %7;
}

QMenu::item:disabled {
    color: %8;
}

QMenu::separator {
    height: 1px;
    background-color: %6;
    margin: 4px 8px;
}

QMenu::indicator {
    width: 16px;
    height: 16px;
    margin-left: 4px;
}

/* ========================================================================== */
/* QToolBar                                                                   */
/* ========================================================================== */

QToolBar {
    background-color: %2;
    border: none;
    padding: 2px;
    spacing: 2px;
}

QToolBar::separator {
    background-color: %6;
    width: 1px;
    margin: 4px 2px;
}

QToolButton {
    background-color: transparent;
    border: 1px solid transparent;
    border-radius: 2px;
    padding: 4px;
}

QToolButton:hover {
    background-color: %7;
    border-color: %6;
}

QToolButton:pressed {
    background-color: %9;
}

QToolButton:checked {
    background-color: %3;
    border-color: %3;
}

/* ========================================================================== */
/* QDockWidget                                                                */
/* ========================================================================== */

QDockWidget {
    titlebar-close-icon: url(:/icons/close.svg);
    titlebar-normal-icon: url(:/icons/float.svg);
}

QDockWidget::title {
    background-color: %2;
    padding: 6px;
    border-bottom: 1px solid %6;
    text-align: left;
}

QDockWidget::close-button,
QDockWidget::float-button {
    background-color: transparent;
    border: none;
    padding: 2px;
}

QDockWidget::close-button:hover,
QDockWidget::float-button:hover {
    background-color: %7;
}

/* ========================================================================== */
/* QTabWidget / QTabBar                                                       */
/* ========================================================================== */

QTabWidget::pane {
    border: 1px solid %6;
    background-color: %2;
}

QTabBar::tab {
    background-color: %5;
    border: 1px solid %6;
    border-bottom: none;
    padding: 6px 12px;
    margin-right: 2px;
}

QTabBar::tab:selected {
    background-color: %2;
    border-bottom: 2px solid %3;
}

QTabBar::tab:hover:!selected {
    background-color: %7;
}

/* ========================================================================== */
/* QPushButton                                                                */
/* ========================================================================== */

QPushButton {
    background-color: %10;
    border: 1px solid %6;
    border-radius: 3px;
    padding: 6px 16px;
    min-width: 80px;
}

QPushButton:hover {
    background-color: %7;
    border-color: %3;
}

QPushButton:pressed {
    background-color: %9;
}

QPushButton:disabled {
    background-color: %5;
    color: %8;
}

QPushButton:default {
    border-color: %3;
}

/* ========================================================================== */
/* QLineEdit                                                                  */
/* ========================================================================== */

QLineEdit {
    background-color: %5;
    border: 1px solid %6;
    border-radius: 3px;
    padding: 4px 8px;
    selection-background-color: %3;
}

QLineEdit:focus {
    border-color: %3;
}

QLineEdit:disabled {
    background-color: %2;
    color: %8;
}

/* ========================================================================== */
/* QTextEdit / QPlainTextEdit                                                 */
/* ========================================================================== */

QTextEdit, QPlainTextEdit {
    background-color: %5;
    border: 1px solid %6;
    border-radius: 3px;
    selection-background-color: %3;
}

QTextEdit:focus, QPlainTextEdit:focus {
    border-color: %3;
}

/* ========================================================================== */
/* QTreeView / QListView / QTableView                                         */
/* ========================================================================== */

QTreeView, QListView, QTableView {
    background-color: %5;
    border: 1px solid %6;
    alternate-background-color: %2;
    selection-background-color: %3;
}

QTreeView::item, QListView::item, QTableView::item {
    padding: 4px;
}

QTreeView::item:hover, QListView::item:hover, QTableView::item:hover {
    background-color: %7;
}

QTreeView::item:selected, QListView::item:selected, QTableView::item:selected {
    background-color: %3;
}

QTreeView::branch:has-children:!has-siblings:closed,
QTreeView::branch:closed:has-children:has-siblings {
    border-image: none;
    image: url(:/icons/branch-closed.svg);
}

QTreeView::branch:open:has-children:!has-siblings,
QTreeView::branch:open:has-children:has-siblings {
    border-image: none;
    image: url(:/icons/branch-open.svg);
}

QHeaderView::section {
    background-color: %2;
    border: none;
    border-right: 1px solid %6;
    border-bottom: 1px solid %6;
    padding: 4px 8px;
}

QHeaderView::section:hover {
    background-color: %7;
}

/* ========================================================================== */
/* QScrollBar                                                                 */
/* ========================================================================== */

QScrollBar:vertical {
    background-color: %5;
    width: 12px;
    margin: 0;
}

QScrollBar:horizontal {
    background-color: %5;
    height: 12px;
    margin: 0;
}

QScrollBar::handle:vertical, QScrollBar::handle:horizontal {
    background-color: %6;
    border-radius: 3px;
    min-height: 30px;
    min-width: 30px;
    margin: 2px;
}

QScrollBar::handle:vertical:hover, QScrollBar::handle:horizontal:hover {
    background-color: %11;
}

QScrollBar::add-line, QScrollBar::sub-line {
    height: 0;
    width: 0;
}

QScrollBar::add-page, QScrollBar::sub-page {
    background: none;
}

/* ========================================================================== */
/* QSplitter                                                                  */
/* ========================================================================== */

QSplitter::handle {
    background-color: %6;
}

QSplitter::handle:hover {
    background-color: %3;
}

QSplitter::handle:horizontal {
    width: 2px;
}

QSplitter::handle:vertical {
    height: 2px;
}

/* ========================================================================== */
/* QComboBox                                                                  */
/* ========================================================================== */

QComboBox {
    background-color: %5;
    border: 1px solid %6;
    border-radius: 3px;
    padding: 4px 8px;
    min-width: 100px;
}

QComboBox:hover {
    border-color: %3;
}

QComboBox::drop-down {
    border: none;
    width: 20px;
}

QComboBox::down-arrow {
    image: url(:/icons/dropdown.svg);
    width: 12px;
    height: 12px;
}

QComboBox QAbstractItemView {
    background-color: %2;
    border: 1px solid %6;
    selection-background-color: %3;
}

/* ========================================================================== */
/* QSpinBox / QDoubleSpinBox                                                  */
/* ========================================================================== */

QSpinBox, QDoubleSpinBox {
    background-color: %5;
    border: 1px solid %6;
    border-radius: 3px;
    padding: 4px;
}

QSpinBox:focus, QDoubleSpinBox:focus {
    border-color: %3;
}

QSpinBox::up-button, QDoubleSpinBox::up-button {
    border: none;
    background-color: transparent;
    width: 16px;
}

QSpinBox::down-button, QDoubleSpinBox::down-button {
    border: none;
    background-color: transparent;
    width: 16px;
}

/* ========================================================================== */
/* QSlider                                                                    */
/* ========================================================================== */

QSlider::groove:horizontal {
    height: 4px;
    background-color: %6;
    border-radius: 2px;
}

QSlider::handle:horizontal {
    background-color: %3;
    width: 14px;
    height: 14px;
    margin: -5px 0;
    border-radius: 7px;
}

QSlider::handle:horizontal:hover {
    background-color: %12;
}

/* ========================================================================== */
/* QProgressBar                                                               */
/* ========================================================================== */

QProgressBar {
    background-color: %5;
    border: 1px solid %6;
    border-radius: 3px;
    text-align: center;
}

QProgressBar::chunk {
    background-color: %3;
    border-radius: 2px;
}

/* ========================================================================== */
/* QGroupBox                                                                  */
/* ========================================================================== */

QGroupBox {
    border: 1px solid %6;
    border-radius: 3px;
    margin-top: 8px;
    padding-top: 8px;
}

QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    padding: 0 4px;
    color: %13;
}

/* ========================================================================== */
/* QToolTip                                                                   */
/* ========================================================================== */

QToolTip {
    background-color: %2;
    border: 1px solid %6;
    color: %1;
    padding: 4px 8px;
}

/* ========================================================================== */
/* QStatusBar                                                                 */
/* ========================================================================== */

QStatusBar {
    background-color: %2;
    border-top: 1px solid %6;
}

QStatusBar::item {
    border: none;
}

)")
      .arg(colorToStyleString(p.textPrimary))    // %1
      .arg(colorToStyleString(p.bgDark))         // %2
      .arg(colorToStyleString(p.accentPrimary))  // %3
      .arg(colorToStyleString(p.textPrimary))    // %4
      .arg(colorToStyleString(p.bgDarkest))      // %5
      .arg(colorToStyleString(p.borderLight))    // %6
      .arg(colorToStyleString(p.bgLight))        // %7
      .arg(colorToStyleString(p.textDisabled))   // %8
      .arg(colorToStyleString(p.accentActive))   // %9
      .arg(colorToStyleString(p.bgMedium))       // %10
      .arg(colorToStyleString(p.textSecondary))  // %11
      .arg(colorToStyleString(p.accentHover))    // %12
      .arg(colorToStyleString(p.textSecondary)); // %13
}

} // namespace NovelMind::editor::qt
