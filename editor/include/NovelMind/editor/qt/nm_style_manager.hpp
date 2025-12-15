#pragma once

/**
 * @file nm_style_manager.hpp
 * @brief Style management for NovelMind Editor
 *
 * Provides Unreal Engine-like dark theme styling using Qt Style Sheets (QSS).
 * Manages:
 * - Application-wide dark theme
 * - High-DPI scaling
 * - Custom color palette
 * - Consistent widget styling
 */

#include <QApplication>
#include <QColor>
#include <QFont>
#include <QObject>
#include <QString>

namespace NovelMind::editor::qt {

/**
 * @brief Color palette for the editor theme
 */
struct EditorPalette {
  // Background colors
  QColor bgDarkest{0x1a, 0x1a, 0x1a}; // Main background
  QColor bgDark{0x23, 0x23, 0x23};    // Panel backgrounds
  QColor bgMedium{0x2d, 0x2d, 0x2d};  // Widget backgrounds
  QColor bgLight{0x38, 0x38, 0x38};   // Hover states

  // Text colors
  QColor textPrimary{0xe0, 0xe0, 0xe0};   // Primary text
  QColor textSecondary{0xa0, 0xa0, 0xa0}; // Secondary text
  QColor textDisabled{0x60, 0x60, 0x60};  // Disabled text

  // Accent colors
  QColor accentPrimary{0x00, 0x78, 0xd4}; // Selection, focus
  QColor accentHover{0x1a, 0x88, 0xe0};   // Hover state
  QColor accentActive{0x00, 0x6c, 0xbd};  // Active state

  // Status colors
  QColor error{0xf4, 0x43, 0x36};
  QColor warning{0xff, 0x98, 0x00};
  QColor success{0x4c, 0xaf, 0x50};
  QColor info{0x21, 0x96, 0xf3};

  // Border colors
  QColor borderDark{0x1a, 0x1a, 0x1a};
  QColor borderLight{0x40, 0x40, 0x40};

  // Graph/Node specific colors
  QColor nodeDefault{0x35, 0x35, 0x35};
  QColor nodeSelected{0x00, 0x78, 0xd4};
  QColor nodeHover{0x40, 0x40, 0x40};
  QColor connectionLine{0x80, 0x80, 0x80};
  QColor gridLine{0x30, 0x30, 0x30};
  QColor gridMajor{0x40, 0x40, 0x40};
};

/**
 * @brief Manages the editor's visual style and theme
 */
class NMStyleManager : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Get the singleton instance
   */
  static NMStyleManager &instance();

  /**
   * @brief Initialize the style manager and apply the default theme
   * @param app The QApplication instance
   */
  void initialize(QApplication *app);

  /**
   * @brief Apply the dark theme to the application
   */
  void applyDarkTheme();

  /**
   * @brief Get the current color palette
   */
  [[nodiscard]] const EditorPalette &palette() const { return m_palette; }

  /**
   * @brief Get the default font for the editor
   */
  [[nodiscard]] QFont defaultFont() const { return m_defaultFont; }

  /**
   * @brief Get the monospace font (for code/console)
   */
  [[nodiscard]] QFont monospaceFont() const { return m_monospaceFont; }

  /**
   * @brief Get the icon size for toolbars
   */
  [[nodiscard]] int toolbarIconSize() const { return m_toolbarIconSize; }

  /**
   * @brief Get the icon size for menus
   */
  [[nodiscard]] int menuIconSize() const { return m_menuIconSize; }

  /**
   * @brief Set the UI scale factor (for high-DPI support)
   * @param scale Scale factor (1.0 = 100%, 1.5 = 150%, etc.)
   */
  void setUiScale(double scale);

  /**
   * @brief Get the current UI scale factor
   */
  [[nodiscard]] double uiScale() const { return m_uiScale; }

  /**
   * @brief Get the complete stylesheet for the application
   */
  [[nodiscard]] QString getStyleSheet() const;

  /**
   * @brief Convert a color to a CSS-compatible string
   */
  static QString colorToStyleString(const QColor &color);

  /**
   * @brief Convert a color with alpha to a CSS rgba string
   */
  static QString colorToRgbaString(const QColor &color, int alpha = 255);

signals:
  /**
   * @brief Emitted when the theme changes
   */
  void themeChanged();

  /**
   * @brief Emitted when the UI scale changes
   */
  void scaleChanged(double newScale);

private:
  NMStyleManager();
  ~NMStyleManager() override = default;

  // Prevent copying
  NMStyleManager(const NMStyleManager &) = delete;
  NMStyleManager &operator=(const NMStyleManager &) = delete;

  void setupFonts();
  void setupHighDpi();

  QApplication *m_app = nullptr;
  EditorPalette m_palette;
  QFont m_defaultFont;
  QFont m_monospaceFont;
  double m_uiScale = 1.0;
  int m_toolbarIconSize = 24;
  int m_menuIconSize = 16;
};

} // namespace NovelMind::editor::qt
