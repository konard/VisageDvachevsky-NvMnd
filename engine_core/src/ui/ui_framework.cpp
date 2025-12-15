#include "NovelMind/ui/ui_framework.hpp"
#include <algorithm>

namespace NovelMind::ui {

// ============================================================================
// Theme Implementation
// ============================================================================

Theme::Theme() {
  // Set default style
  m_defaultStyle = Style{};
}

void Theme::setStyle(const std::string &name, const Style &style) {
  m_styles[name] = style;
}

const Style &Theme::getStyle(const std::string &name) const {
  auto it = m_styles.find(name);
  return (it != m_styles.end()) ? it->second : m_defaultStyle;
}

bool Theme::hasStyle(const std::string &name) const {
  return m_styles.find(name) != m_styles.end();
}

Theme Theme::createDarkTheme() {
  Theme theme;

  // Default style
  Style defaultStyle;
  defaultStyle.backgroundColor = renderer::Color{40, 40, 40, 255};
  defaultStyle.foregroundColor = renderer::Color{255, 255, 255, 255};
  defaultStyle.borderColor = renderer::Color{80, 80, 80, 255};
  defaultStyle.textColor = renderer::Color{220, 220, 220, 255};
  theme.setStyle("default", defaultStyle);

  // Button style
  Style buttonStyle = defaultStyle;
  buttonStyle.backgroundColor = renderer::Color{60, 60, 60, 255};
  buttonStyle.hoverColor = renderer::Color{80, 80, 80, 255};
  buttonStyle.activeColor = renderer::Color{100, 100, 100, 255};
  buttonStyle.borderWidth = 1.0f;
  buttonStyle.borderRadius = 4.0f;
  buttonStyle.padding = Insets::symmetric(16.0f, 8.0f);
  theme.setStyle("button", buttonStyle);

  // Label style
  Style labelStyle = defaultStyle;
  labelStyle.backgroundColor = renderer::Color{0, 0, 0, 0};
  labelStyle.padding = Insets::all(4.0f);
  theme.setStyle("label", labelStyle);

  // Panel style
  Style panelStyle = defaultStyle;
  panelStyle.backgroundColor = renderer::Color{50, 50, 50, 255};
  panelStyle.borderWidth = 1.0f;
  panelStyle.borderColor = renderer::Color{70, 70, 70, 255};
  panelStyle.borderRadius = 4.0f;
  panelStyle.padding = Insets::all(8.0f);
  theme.setStyle("panel", panelStyle);

  // Input style
  Style inputStyle = defaultStyle;
  inputStyle.backgroundColor = renderer::Color{30, 30, 30, 255};
  inputStyle.borderWidth = 1.0f;
  inputStyle.borderColor = renderer::Color{80, 80, 80, 255};
  inputStyle.borderRadius = 4.0f;
  inputStyle.padding = Insets::symmetric(8.0f, 6.0f);
  theme.setStyle("input", inputStyle);

  // Checkbox style
  Style checkboxStyle = defaultStyle;
  checkboxStyle.backgroundColor = renderer::Color{0, 0, 0, 0};
  checkboxStyle.accentColor = renderer::Color{0, 120, 215, 255};
  theme.setStyle("checkbox", checkboxStyle);

  // Slider style
  Style sliderStyle = defaultStyle;
  sliderStyle.backgroundColor = renderer::Color{60, 60, 60, 255};
  sliderStyle.accentColor = renderer::Color{0, 120, 215, 255};
  sliderStyle.borderRadius = 4.0f;
  theme.setStyle("slider", sliderStyle);

  return theme;
}

Theme Theme::createLightTheme() {
  Theme theme;

  // Default style
  Style defaultStyle;
  defaultStyle.backgroundColor = renderer::Color{245, 245, 245, 255};
  defaultStyle.foregroundColor = renderer::Color{0, 0, 0, 255};
  defaultStyle.borderColor = renderer::Color{200, 200, 200, 255};
  defaultStyle.textColor = renderer::Color{30, 30, 30, 255};
  theme.setStyle("default", defaultStyle);

  // Button style
  Style buttonStyle = defaultStyle;
  buttonStyle.backgroundColor = renderer::Color{225, 225, 225, 255};
  buttonStyle.hoverColor = renderer::Color{210, 210, 210, 255};
  buttonStyle.activeColor = renderer::Color{195, 195, 195, 255};
  buttonStyle.borderWidth = 1.0f;
  buttonStyle.borderRadius = 4.0f;
  buttonStyle.padding = Insets::symmetric(16.0f, 8.0f);
  theme.setStyle("button", buttonStyle);

  // Label style
  Style labelStyle = defaultStyle;
  labelStyle.backgroundColor = renderer::Color{0, 0, 0, 0};
  labelStyle.padding = Insets::all(4.0f);
  theme.setStyle("label", labelStyle);

  // Panel style
  Style panelStyle = defaultStyle;
  panelStyle.backgroundColor = renderer::Color{255, 255, 255, 255};
  panelStyle.borderWidth = 1.0f;
  panelStyle.borderColor = renderer::Color{220, 220, 220, 255};
  panelStyle.borderRadius = 4.0f;
  panelStyle.padding = Insets::all(8.0f);
  theme.setStyle("panel", panelStyle);

  // Input style
  Style inputStyle = defaultStyle;
  inputStyle.backgroundColor = renderer::Color{255, 255, 255, 255};
  inputStyle.borderWidth = 1.0f;
  inputStyle.borderColor = renderer::Color{180, 180, 180, 255};
  inputStyle.borderRadius = 4.0f;
  inputStyle.padding = Insets::symmetric(8.0f, 6.0f);
  theme.setStyle("input", inputStyle);

  return theme;
}

// ============================================================================
// Widget Implementation
// ============================================================================

Widget::Widget(const std::string &id) : m_id(id) {}

void Widget::setParent(Widget *parent) { m_parent = parent; }

void Widget::setBounds(const Rect &bounds) { m_bounds = bounds; }

void Widget::setPosition(f32 x, f32 y) {
  m_bounds.x = x;
  m_bounds.y = y;
}

void Widget::setSize(f32 width, f32 height) {
  m_bounds.width = width;
  m_bounds.height = height;
}

void Widget::setConstraints(const SizeConstraints &constraints) {
  m_constraints = constraints;
}

void Widget::setAlignment(Alignment horizontal, Alignment vertical) {
  m_horizontalAlign = horizontal;
  m_verticalAlign = vertical;
}

void Widget::setVisible(bool visible) { m_visible = visible; }

void Widget::setEnabled(bool enabled) { m_enabled = enabled; }

void Widget::setStyle(const Style &style) { m_style = style; }

void Widget::requestFocus() {
  // Request through parent or UIManager
  m_focused = true;
}

void Widget::releaseFocus() { m_focused = false; }

void Widget::on(UIEventType type, EventHandler handler) {
  m_eventHandlers[type] = std::move(handler);
}

void Widget::off(UIEventType type) { m_eventHandlers.erase(type); }

void Widget::update(f64 /*deltaTime*/) {
  // Base implementation does nothing
}

void Widget::render(renderer::IRenderer &renderer) {
  if (!m_visible) {
    return;
  }

  renderBackground(renderer);
}

void Widget::layout() {
  // Base implementation does nothing
}

bool Widget::handleEvent(UIEvent &event) {
  if (!m_visible || !m_enabled) {
    return false;
  }

  // Update hover state
  if (event.type == UIEventType::MouseEnter) {
    m_hovered = true;
  } else if (event.type == UIEventType::MouseLeave) {
    m_hovered = false;
  }

  // Update pressed state
  if (event.type == UIEventType::MouseDown) {
    m_pressed = true;
  } else if (event.type == UIEventType::MouseUp) {
    m_pressed = false;
  }

  fireEvent(event);
  return event.consumed;
}

Rect Widget::measure(f32 availableWidth, f32 availableHeight) {
  f32 width = m_constraints.preferredWidth > 0
                  ? m_constraints.preferredWidth
                  : std::min(availableWidth, m_constraints.maxWidth);
  f32 height = m_constraints.preferredHeight > 0
                   ? m_constraints.preferredHeight
                   : std::min(availableHeight, m_constraints.maxHeight);

  width = std::max(width, m_constraints.minWidth);
  height = std::max(height, m_constraints.minHeight);

  return {0, 0, width, height};
}

void Widget::fireEvent(UIEvent &event) {
  auto it = m_eventHandlers.find(event.type);
  if (it != m_eventHandlers.end() && it->second) {
    it->second(event);
  }
}

void Widget::renderBackground(renderer::IRenderer &renderer) {
  renderer::Color bgColor = m_style.backgroundColor;

  if (!m_enabled) {
    bgColor = m_style.disabledColor;
  } else if (m_pressed) {
    bgColor = m_style.activeColor;
  } else if (m_hovered) {
    bgColor = m_style.hoverColor;
  }

  bgColor.a = static_cast<u8>(bgColor.a * m_style.opacity);

  if (bgColor.a > 0) {
    renderer::Rect rect{m_bounds.x, m_bounds.y, m_bounds.width,
                        m_bounds.height};
    renderer.fillRect(rect, bgColor);
  }

  // Draw border
  if (m_style.borderWidth > 0) {
    renderer::Color borderColor = m_style.borderColor;
    borderColor.a = static_cast<u8>(borderColor.a * m_style.opacity);
    // Border rendering would go here
  }
}

// ============================================================================
// Container Implementation
// ============================================================================

Container::Container(const std::string &id) : Widget(id) {}

void Container::addChild(std::shared_ptr<Widget> child) {
  if (child) {
    child->setParent(this);
    m_children.push_back(std::move(child));
  }
}

void Container::removeChild(const std::string &id) {
  m_children.erase(
      std::remove_if(m_children.begin(), m_children.end(),
                     [&id](const auto &child) { return child->getId() == id; }),
      m_children.end());
}

void Container::removeChild(Widget *child) {
  m_children.erase(
      std::remove_if(m_children.begin(), m_children.end(),
                     [child](const auto &c) { return c.get() == child; }),
      m_children.end());
}

void Container::clearChildren() { m_children.clear(); }

Widget *Container::findChild(const std::string &id) {
  for (auto &child : m_children) {
    if (child->getId() == id) {
      return child.get();
    }

    // Recursive search in containers
    if (auto *container = dynamic_cast<Container *>(child.get())) {
      if (auto *found = container->findChild(id)) {
        return found;
      }
    }
  }
  return nullptr;
}

void Container::update(f64 deltaTime) {
  Widget::update(deltaTime);

  for (auto &child : m_children) {
    if (child->isVisible()) {
      child->update(deltaTime);
    }
  }
}

void Container::render(renderer::IRenderer &renderer) {
  if (!m_visible) {
    return;
  }

  Widget::render(renderer);

  for (auto &child : m_children) {
    if (child->isVisible()) {
      child->render(renderer);
    }
  }
}

void Container::layout() { layoutChildren(); }

bool Container::handleEvent(UIEvent &event) {
  if (!m_visible || !m_enabled) {
    return false;
  }

  // Reverse order for proper z-order handling
  for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
    if ((*it)->isVisible() && (*it)->handleEvent(event)) {
      return true;
    }
  }

  return Widget::handleEvent(event);
}

Rect Container::measure(f32 availableWidth, f32 availableHeight) {
  f32 contentWidth = 0.0f;
  f32 contentHeight = 0.0f;

  for (auto &child : m_children) {
    if (!child->isVisible()) {
      continue;
    }

    Rect childSize = child->measure(availableWidth, availableHeight);

    if (m_layoutDirection == LayoutDirection::Horizontal) {
      contentWidth += childSize.width + m_spacing;
      contentHeight = std::max(contentHeight, childSize.height);
    } else {
      contentWidth = std::max(contentWidth, childSize.width);
      contentHeight += childSize.height + m_spacing;
    }
  }

  // Remove extra spacing
  if (!m_children.empty()) {
    if (m_layoutDirection == LayoutDirection::Horizontal) {
      contentWidth -= m_spacing;
    } else {
      contentHeight -= m_spacing;
    }
  }

  // Add padding
  contentWidth += m_style.padding.left + m_style.padding.right;
  contentHeight += m_style.padding.top + m_style.padding.bottom;

  return {0, 0, contentWidth, contentHeight};
}

void Container::layoutChildren() {
  // Base implementation - simple stacking
  f32 x = m_bounds.x + m_style.padding.left;
  f32 y = m_bounds.y + m_style.padding.top;

  for (auto &child : m_children) {
    if (!child->isVisible()) {
      continue;
    }

    Rect measured = child->measure(
        m_bounds.width - m_style.padding.left - m_style.padding.right,
        m_bounds.height - m_style.padding.top - m_style.padding.bottom);

    child->setBounds({x, y, measured.width, measured.height});
    child->layout();

    if (m_layoutDirection == LayoutDirection::Horizontal) {
      x += measured.width + m_spacing;
    } else {
      y += measured.height + m_spacing;
    }
  }
}

// ============================================================================
// HBox Implementation
// ============================================================================

HBox::HBox(const std::string &id) : Container(id) {
  m_layoutDirection = LayoutDirection::Horizontal;
}

void HBox::layoutChildren() {
  f32 availableWidth =
      m_bounds.width - m_style.padding.left - m_style.padding.right;
  f32 availableHeight =
      m_bounds.height - m_style.padding.top - m_style.padding.bottom;

  // Calculate total flex grow
  f32 totalFlexGrow = 0.0f;
  f32 fixedWidth = 0.0f;

  for (auto &child : m_children) {
    if (!child->isVisible()) {
      continue;
    }

    if (child->getFlexGrow() > 0.0f) {
      totalFlexGrow += child->getFlexGrow();
    } else {
      Rect measured = child->measure(availableWidth, availableHeight);
      fixedWidth += measured.width;
    }
  }

  // Account for spacing
  f32 spacing =
      m_spacing *
      static_cast<f32>(
          std::count_if(m_children.begin(), m_children.end(),
                        [](const auto &c) { return c->isVisible(); }) -
          1);
  f32 flexSpace = std::max(0.0f, availableWidth - fixedWidth - spacing);

  // Layout children
  f32 x = m_bounds.x + m_style.padding.left;
  f32 y = m_bounds.y + m_style.padding.top;

  for (auto &child : m_children) {
    if (!child->isVisible()) {
      continue;
    }

    f32 childWidth;
    if (child->getFlexGrow() > 0.0f && totalFlexGrow > 0.0f) {
      childWidth = flexSpace * (child->getFlexGrow() / totalFlexGrow);
    } else {
      Rect measured = child->measure(availableWidth, availableHeight);
      childWidth = measured.width;
    }

    f32 childHeight = availableHeight;
    f32 childY = y;

    // Vertical alignment
    if (child->getVerticalAlignment() != Alignment::Stretch) {
      Rect measured = child->measure(childWidth, availableHeight);
      childHeight = measured.height;

      switch (child->getVerticalAlignment()) {
      case Alignment::Center:
        childY = y + (availableHeight - childHeight) / 2.0f;
        break;
      case Alignment::End:
        childY = y + availableHeight - childHeight;
        break;
      default:
        break;
      }
    }

    child->setBounds({x, childY, childWidth, childHeight});
    child->layout();

    x += childWidth + m_spacing;
  }
}

// ============================================================================
// VBox Implementation
// ============================================================================

VBox::VBox(const std::string &id) : Container(id) {
  m_layoutDirection = LayoutDirection::Vertical;
}

void VBox::layoutChildren() {
  f32 availableWidth =
      m_bounds.width - m_style.padding.left - m_style.padding.right;
  f32 availableHeight =
      m_bounds.height - m_style.padding.top - m_style.padding.bottom;

  // Calculate total flex grow
  f32 totalFlexGrow = 0.0f;
  f32 fixedHeight = 0.0f;

  for (auto &child : m_children) {
    if (!child->isVisible()) {
      continue;
    }

    if (child->getFlexGrow() > 0.0f) {
      totalFlexGrow += child->getFlexGrow();
    } else {
      Rect measured = child->measure(availableWidth, availableHeight);
      fixedHeight += measured.height;
    }
  }

  // Account for spacing
  f32 spacing =
      m_spacing *
      static_cast<f32>(
          std::count_if(m_children.begin(), m_children.end(),
                        [](const auto &c) { return c->isVisible(); }) -
          1);
  f32 flexSpace = std::max(0.0f, availableHeight - fixedHeight - spacing);

  // Layout children
  f32 x = m_bounds.x + m_style.padding.left;
  f32 y = m_bounds.y + m_style.padding.top;

  for (auto &child : m_children) {
    if (!child->isVisible()) {
      continue;
    }

    f32 childHeight;
    if (child->getFlexGrow() > 0.0f && totalFlexGrow > 0.0f) {
      childHeight = flexSpace * (child->getFlexGrow() / totalFlexGrow);
    } else {
      Rect measured = child->measure(availableWidth, availableHeight);
      childHeight = measured.height;
    }

    f32 childWidth = availableWidth;
    f32 childX = x;

    // Horizontal alignment
    if (child->getHorizontalAlignment() != Alignment::Stretch) {
      Rect measured = child->measure(availableWidth, childHeight);
      childWidth = measured.width;

      switch (child->getHorizontalAlignment()) {
      case Alignment::Center:
        childX = x + (availableWidth - childWidth) / 2.0f;
        break;
      case Alignment::End:
        childX = x + availableWidth - childWidth;
        break;
      default:
        break;
      }
    }

    child->setBounds({childX, y, childWidth, childHeight});
    child->layout();

    y += childHeight + m_spacing;
  }
}

// ============================================================================
// Grid Implementation
// ============================================================================

Grid::Grid(const std::string &id) : Container(id) {}

void Grid::layoutChildren() {
  if (m_columns <= 0 || m_children.empty()) {
    return;
  }

  f32 availableWidth =
      m_bounds.width - m_style.padding.left - m_style.padding.right;
  f32 availableHeight =
      m_bounds.height - m_style.padding.top - m_style.padding.bottom;

  f32 cellWidth =
      (availableWidth - m_columnSpacing * static_cast<f32>(m_columns - 1)) /
      static_cast<f32>(m_columns);

  // Calculate row heights
  std::vector<f32> rowHeights;
  size_t visibleCount = static_cast<size_t>(
      std::count_if(m_children.begin(), m_children.end(),
                    [](const auto &c) { return c->isVisible(); }));
  i32 rows = (static_cast<i32>(visibleCount) + m_columns - 1) / m_columns;

  for (i32 row = 0; row < rows; ++row) {
    f32 maxHeight = 0.0f;
    for (i32 col = 0; col < m_columns; ++col) {
      size_t idx = static_cast<size_t>(row * m_columns + col);
      if (idx >= m_children.size() || !m_children[idx]->isVisible()) {
        continue;
      }

      Rect measured = m_children[idx]->measure(cellWidth, availableHeight);
      maxHeight = std::max(maxHeight, measured.height);
    }
    rowHeights.push_back(maxHeight);
  }

  // Layout
  f32 y = m_bounds.y + m_style.padding.top;
  size_t idx = 0;

  for (i32 row = 0; row < rows; ++row) {
    f32 x = m_bounds.x + m_style.padding.left;

    for (i32 col = 0; col < m_columns; ++col) {
      // Find next visible child
      while (idx < m_children.size() && !m_children[idx]->isVisible()) {
        ++idx;
      }

      if (idx >= m_children.size()) {
        break;
      }

      m_children[idx]->setBounds(
          {x, y, cellWidth, rowHeights[static_cast<size_t>(row)]});
      m_children[idx]->layout();

      x += cellWidth + m_columnSpacing;
      ++idx;
    }

    y += rowHeights[static_cast<size_t>(row)] + m_rowSpacing;
  }
}

// ============================================================================
// Label Implementation
// ============================================================================

Label::Label(const std::string &text, const std::string &id)
    : Widget(id), m_text(text) {}

void Label::setText(const std::string &text) { m_text = text; }

void Label::render(renderer::IRenderer &renderer) {
  if (!m_visible) {
    return;
  }

  Widget::render(renderer);

  renderer::Color textColor = m_style.textColor;
  textColor.a = static_cast<u8>(textColor.a * m_style.opacity);

  // Text rendering is handled when a TextRenderer is available.
  // The textRenderer.drawText() call uses the computed textColor.
  // Font/text renderer integration is managed by the application layer.
  (void)textColor; // Reserved for text renderer integration
}

Rect Label::measure(f32 /*availableWidth*/, f32 /*availableHeight*/) {
  // Simplified text measurement
  f32 charWidth = m_style.fontSize * 0.6f;
  f32 width = static_cast<f32>(m_text.length()) * charWidth +
              m_style.padding.left + m_style.padding.right;
  f32 height = m_style.fontSize + m_style.padding.top + m_style.padding.bottom;

  return {0, 0, width, height};
}

// ============================================================================
// Button Implementation
// ============================================================================

Button::Button(const std::string &text, const std::string &id)
    : Widget(id), m_text(text) {
  m_focusable = true;
}

void Button::render(renderer::IRenderer &renderer) {
  if (!m_visible) {
    return;
  }

  Widget::render(renderer);

  renderer::Color textColor = m_style.textColor;
  if (!m_enabled) {
    textColor.a /= 2;
  }
  textColor.a = static_cast<u8>(textColor.a * m_style.opacity);

  // Center text in button
  f32 charWidth = m_style.fontSize * 0.6f;
  f32 textWidth = static_cast<f32>(m_text.length()) * charWidth;
  f32 textHeight = m_style.fontSize;

  f32 textX = m_bounds.x + (m_bounds.width - textWidth) / 2.0f;
  f32 textY = m_bounds.y + (m_bounds.height - textHeight) / 2.0f;

  // Text rendering is handled when a TextRenderer is available.
  // Center coordinates are pre-calculated for text placement.
  (void)textX;     // Reserved for text renderer
  (void)textY;     // Reserved for text renderer
  (void)textColor; // Reserved for text renderer
}

bool Button::handleEvent(UIEvent &event) {
  Widget::handleEvent(event);

  if (event.type == UIEventType::Click && m_enabled && m_onClick) {
    m_onClick();
    event.consume();
    return true;
  }

  return false;
}

Rect Button::measure(f32 /*availableWidth*/, f32 /*availableHeight*/) {
  f32 charWidth = m_style.fontSize * 0.6f;
  f32 width = static_cast<f32>(m_text.length()) * charWidth +
              m_style.padding.left + m_style.padding.right;
  f32 height = m_style.fontSize + m_style.padding.top + m_style.padding.bottom;

  width = std::max(width, m_constraints.minWidth);
  height = std::max(height, m_constraints.minHeight);

  return {0, 0, width, height};
}

// ============================================================================
// TextInput Implementation
// ============================================================================

TextInput::TextInput(const std::string &id) : Widget(id) { m_focusable = true; }

void TextInput::setText(const std::string &text) {
  m_text = text.substr(0, m_maxLength);
  m_cursorPos = m_text.length();
}

void TextInput::render(renderer::IRenderer &renderer) {
  if (!m_visible) {
    return;
  }

  Widget::render(renderer);

  std::string displayText = m_text;
  if (m_password) {
    displayText = std::string(m_text.length(), '*');
  }

  renderer::Color textColor = m_style.textColor;
  if (m_text.empty() && !m_placeholder.empty()) {
    displayText = m_placeholder;
    textColor.a /= 2;
  }

  textColor.a = static_cast<u8>(textColor.a * m_style.opacity);

  // Text rendering is handled when a TextRenderer is available.
  // Display text accounts for password masking and placeholder state.
  (void)displayText; // Reserved for text renderer
  (void)textColor;   // Reserved for text renderer

  // Draw cursor
  if (m_focused) {
    f32 cursorX = m_bounds.x + m_style.padding.left +
                  static_cast<f32>(m_cursorPos) * m_style.fontSize * 0.6f -
                  m_scrollOffset;
    if (static_cast<i32>(m_cursorBlink * 2) % 2 == 0) {
      renderer::Color cursorColor{255, 255, 255, 255};
      renderer::Rect cursorRect{cursorX, m_bounds.y + m_style.padding.top, 2.0f,
                                m_style.fontSize};
      renderer.fillRect(cursorRect, cursorColor);
    }
  }
}

bool TextInput::handleEvent(UIEvent &event) {
  Widget::handleEvent(event);

  if (!m_enabled) {
    return false;
  }

  if (event.type == UIEventType::Click) {
    requestFocus();
    event.consume();
    return true;
  }

  if (m_focused) {
    if (event.type == UIEventType::KeyPress) {
      if (event.character >= 32 && m_text.length() < m_maxLength) {
        m_text.insert(m_cursorPos, 1, event.character);
        ++m_cursorPos;
        if (m_onChange) {
          m_onChange(m_text);
        }
        event.consume();
        return true;
      }
    } else if (event.type == UIEventType::KeyDown) {
      // Handle special keys
      if (event.keyCode == 8) // Backspace
      {
        if (m_cursorPos > 0) {
          m_text.erase(m_cursorPos - 1, 1);
          --m_cursorPos;
          if (m_onChange) {
            m_onChange(m_text);
          }
        }
        event.consume();
        return true;
      } else if (event.keyCode == 127) // Delete
      {
        if (m_cursorPos < m_text.length()) {
          m_text.erase(m_cursorPos, 1);
          if (m_onChange) {
            m_onChange(m_text);
          }
        }
        event.consume();
        return true;
      } else if (event.keyCode == 13) // Enter
      {
        if (m_onSubmit) {
          m_onSubmit(m_text);
        }
        event.consume();
        return true;
      } else if (event.keyCode == 37) // Left arrow
      {
        if (m_cursorPos > 0) {
          --m_cursorPos;
        }
        event.consume();
        return true;
      } else if (event.keyCode == 39) // Right arrow
      {
        if (m_cursorPos < m_text.length()) {
          ++m_cursorPos;
        }
        event.consume();
        return true;
      }
    }
  }

  return false;
}

Rect TextInput::measure(f32 /*availableWidth*/, f32 /*availableHeight*/) {
  f32 width = 200.0f + m_style.padding.left + m_style.padding.right;
  f32 height = m_style.fontSize + m_style.padding.top + m_style.padding.bottom;

  width = std::max(width, m_constraints.minWidth);
  height = std::max(height, m_constraints.minHeight);

  return {0, 0, width, height};
}

// ============================================================================
// Checkbox Implementation
// ============================================================================

Checkbox::Checkbox(const std::string &label, const std::string &id)
    : Widget(id), m_label(label) {
  m_focusable = true;
}

void Checkbox::setChecked(bool checked) {
  if (m_checked != checked) {
    m_checked = checked;
    if (m_onChange) {
      m_onChange(m_checked);
    }
  }
}

void Checkbox::toggle() { setChecked(!m_checked); }

void Checkbox::render(renderer::IRenderer &renderer) {
  if (!m_visible) {
    return;
  }

  Widget::render(renderer);

  f32 boxSize = m_style.fontSize;
  f32 boxX = m_bounds.x + m_style.padding.left;
  f32 boxY = m_bounds.y + (m_bounds.height - boxSize) / 2.0f;

  // Draw checkbox box
  renderer::Color boxColor =
      m_checked ? m_style.accentColor : m_style.backgroundColor;
  renderer::Rect boxRect{boxX, boxY, boxSize, boxSize};
  renderer.fillRect(boxRect, boxColor);

  // Draw border
  renderer::Color borderColor =
      m_hovered ? m_style.accentColor : m_style.borderColor;
  // Border drawing would go here
  (void)borderColor;

  // Draw checkmark if checked
  if (m_checked) {
    renderer::Color checkColor{255, 255, 255, 255};
    // Checkmark drawing would go here
    (void)checkColor;
  }

  // Draw label
  if (!m_label.empty()) {
    renderer::Color textColor = m_style.textColor;
    textColor.a = static_cast<u8>(textColor.a * m_style.opacity);
    // Text rendering is handled when a TextRenderer is available.
    // Label is positioned to the right of the checkbox.
    (void)textColor; // Reserved for text renderer
  }
}

bool Checkbox::handleEvent(UIEvent &event) {
  Widget::handleEvent(event);

  if (event.type == UIEventType::Click && m_enabled) {
    toggle();
    event.consume();
    return true;
  }

  return false;
}

Rect Checkbox::measure(f32 /*availableWidth*/, f32 /*availableHeight*/) {
  f32 boxSize = m_style.fontSize;
  f32 charWidth = m_style.fontSize * 0.6f;
  f32 labelWidth = static_cast<f32>(m_label.length()) * charWidth;

  f32 width = boxSize + 8.0f + labelWidth + m_style.padding.left +
              m_style.padding.right;
  f32 height = std::max(boxSize, m_style.fontSize) + m_style.padding.top +
               m_style.padding.bottom;

  return {0, 0, width, height};
}

// ============================================================================
// Slider Implementation
// ============================================================================

Slider::Slider(const std::string &id) : Widget(id) { m_focusable = true; }

void Slider::setValue(f32 value) {
  f32 newValue = std::max(m_min, std::min(m_max, value));
  if (m_step > 0.0f) {
    newValue = std::round((newValue - m_min) / m_step) * m_step + m_min;
  }

  if (m_value != newValue) {
    m_value = newValue;
    if (m_onChange) {
      m_onChange(m_value);
    }
  }
}

void Slider::setRange(f32 min, f32 max) {
  m_min = min;
  m_max = max;
  setValue(m_value); // Clamp current value
}

void Slider::render(renderer::IRenderer &renderer) {
  if (!m_visible) {
    return;
  }

  Widget::render(renderer);

  f32 trackHeight = 4.0f;
  f32 trackY = m_bounds.y + (m_bounds.height - trackHeight) / 2.0f;
  f32 trackWidth =
      m_bounds.width - m_style.padding.left - m_style.padding.right;
  f32 trackX = m_bounds.x + m_style.padding.left;

  // Draw track
  renderer::Color trackColor = m_style.backgroundColor;
  renderer::Rect trackRect{trackX, trackY, trackWidth, trackHeight};
  renderer.fillRect(trackRect, trackColor);

  // Draw filled portion
  f32 progress = (m_max > m_min) ? (m_value - m_min) / (m_max - m_min) : 0.0f;
  renderer::Color fillColor = m_style.accentColor;
  renderer::Rect fillRect{trackX, trackY, trackWidth * progress, trackHeight};
  renderer.fillRect(fillRect, fillColor);

  // Draw thumb
  f32 thumbSize = 16.0f;
  f32 thumbX = trackX + trackWidth * progress - thumbSize / 2.0f;
  f32 thumbY = m_bounds.y + (m_bounds.height - thumbSize) / 2.0f;

  renderer::Color thumbColor =
      m_dragging || m_hovered ? m_style.hoverColor : m_style.foregroundColor;
  renderer::Rect thumbRect{thumbX, thumbY, thumbSize, thumbSize};
  renderer.fillRect(thumbRect, thumbColor);
}

bool Slider::handleEvent(UIEvent &event) {
  Widget::handleEvent(event);

  if (!m_enabled) {
    return false;
  }

  if (event.type == UIEventType::MouseDown) {
    m_dragging = true;
    // Calculate value from mouse position
    f32 trackWidth =
        m_bounds.width - m_style.padding.left - m_style.padding.right;
    f32 trackX = m_bounds.x + m_style.padding.left;
    f32 progress = (event.mouseX - trackX) / trackWidth;
    setValue(m_min + progress * (m_max - m_min));
    event.consume();
    return true;
  } else if (event.type == UIEventType::MouseMove && m_dragging) {
    f32 trackWidth =
        m_bounds.width - m_style.padding.left - m_style.padding.right;
    f32 trackX = m_bounds.x + m_style.padding.left;
    f32 progress = (event.mouseX - trackX) / trackWidth;
    setValue(m_min + progress * (m_max - m_min));
    event.consume();
    return true;
  } else if (event.type == UIEventType::MouseUp) {
    m_dragging = false;
    event.consume();
    return true;
  }

  return false;
}

Rect Slider::measure(f32 /*availableWidth*/, f32 /*availableHeight*/) {
  f32 width = 200.0f + m_style.padding.left + m_style.padding.right;
  f32 height = 24.0f + m_style.padding.top + m_style.padding.bottom;

  width = std::max(width, m_constraints.minWidth);
  height = std::max(height, m_constraints.minHeight);

  return {0, 0, width, height};
}

// ============================================================================
// ScrollPanel Implementation
// ============================================================================

ScrollPanel::ScrollPanel(const std::string &id) : Container(id) {}

void ScrollPanel::setScrollX(f32 x) {
  m_scrollX = std::max(0.0f, std::min(x, m_contentWidth - m_bounds.width));
}

void ScrollPanel::setScrollY(f32 y) {
  m_scrollY = std::max(0.0f, std::min(y, m_contentHeight - m_bounds.height));
}

void ScrollPanel::render(renderer::IRenderer &renderer) {
  if (!m_visible) {
    return;
  }

  // Render background
  Widget::render(renderer);

  // Clipping is handled when a ClipRect renderer extension is available.
  // The clip region would be set to m_bounds for content containment.

  // Render children with scroll offset
  for (auto &child : m_children) {
    if (child->isVisible()) {
      // Temporarily offset child position
      Rect originalBounds = child->getBounds();
      Rect scrolledBounds = originalBounds;
      scrolledBounds.x -= m_scrollX;
      scrolledBounds.y -= m_scrollY;
      child->setBounds(scrolledBounds);

      child->render(renderer);

      child->setBounds(originalBounds);
    }
  }

  // Clip region restoration is handled when available.

  // Draw scrollbars
  if (m_verticalScroll && m_contentHeight > m_bounds.height) {
    f32 scrollbarWidth = 8.0f;
    f32 scrollbarHeight = (m_bounds.height / m_contentHeight) * m_bounds.height;
    f32 scrollbarY = (m_scrollY / m_contentHeight) * m_bounds.height;

    renderer::Color scrollbarColor{100, 100, 100, 200};
    renderer::Rect scrollbarRect{m_bounds.x + m_bounds.width - scrollbarWidth,
                                 m_bounds.y + scrollbarY, scrollbarWidth,
                                 scrollbarHeight};
    renderer.fillRect(scrollbarRect, scrollbarColor);
  }
}

bool ScrollPanel::handleEvent(UIEvent &event) {
  if (event.type == UIEventType::Scroll) {
    if (m_verticalScroll) {
      setScrollY(m_scrollY - event.deltaY * 30.0f);
    }
    if (m_horizontalScroll) {
      setScrollX(m_scrollX - event.deltaX * 30.0f);
    }
    event.consume();
    return true;
  }

  // Adjust mouse coordinates for scrolling
  UIEvent adjustedEvent = event;
  adjustedEvent.mouseX += m_scrollX;
  adjustedEvent.mouseY += m_scrollY;

  return Container::handleEvent(adjustedEvent);
}

void ScrollPanel::layoutChildren() {
  Container::layoutChildren();

  // Calculate content size
  m_contentWidth = 0.0f;
  m_contentHeight = 0.0f;

  for (const auto &child : m_children) {
    if (!child->isVisible()) {
      continue;
    }

    const Rect &bounds = child->getBounds();
    m_contentWidth =
        std::max(m_contentWidth, bounds.x + bounds.width - m_bounds.x);
    m_contentHeight =
        std::max(m_contentHeight, bounds.y + bounds.height - m_bounds.y);
  }
}

// ============================================================================
// Panel Implementation
// ============================================================================

Panel::Panel(const std::string &id) : Container(id) {}

void Panel::render(renderer::IRenderer &renderer) {
  if (!m_visible) {
    return;
  }

  // Render panel background with border
  Widget::render(renderer);

  // Render children
  for (auto &child : m_children) {
    if (child->isVisible()) {
      child->render(renderer);
    }
  }
}

// ============================================================================
// UIManager Implementation
// ============================================================================

UIManager::UIManager() { m_theme = Theme::createDarkTheme(); }

UIManager::~UIManager() = default;

void UIManager::setRoot(std::shared_ptr<Widget> root) {
  m_root = std::move(root);
  m_layoutDirty = true;
}

void UIManager::setTheme(const Theme &theme) { m_theme = theme; }

void UIManager::setFocus(Widget *widget) {
  if (m_focusedWidget != widget) {
    if (m_focusedWidget) {
      m_focusedWidget->releaseFocus();
      UIEvent blurEvent;
      blurEvent.type = UIEventType::Blur;
      m_focusedWidget->handleEvent(blurEvent);
    }

    m_focusedWidget = widget;

    if (m_focusedWidget) {
      m_focusedWidget->requestFocus();
      UIEvent focusEvent;
      focusEvent.type = UIEventType::Focus;
      m_focusedWidget->handleEvent(focusEvent);
    }
  }
}

void UIManager::clearFocus() { setFocus(nullptr); }

void UIManager::focusNext() {
  std::vector<Widget *> focusable;
  if (m_root) {
    collectFocusableWidgets(m_root.get(), focusable);
  }

  if (focusable.empty()) {
    return;
  }

  auto it = std::find(focusable.begin(), focusable.end(), m_focusedWidget);
  if (it == focusable.end() || ++it == focusable.end()) {
    setFocus(focusable.front());
  } else {
    setFocus(*it);
  }
}

void UIManager::focusPrevious() {
  std::vector<Widget *> focusable;
  if (m_root) {
    collectFocusableWidgets(m_root.get(), focusable);
  }

  if (focusable.empty()) {
    return;
  }

  auto it = std::find(focusable.begin(), focusable.end(), m_focusedWidget);
  if (it == focusable.end() || it == focusable.begin()) {
    setFocus(focusable.back());
  } else {
    setFocus(*--it);
  }
}

void UIManager::pushModal(std::shared_ptr<Widget> modal) {
  m_modalStack.push_back(std::move(modal));
}

void UIManager::popModal() {
  if (!m_modalStack.empty()) {
    m_modalStack.pop_back();
  }
}

void UIManager::update(f64 deltaTime) {
  if (m_layoutDirty) {
    performLayout();
  }

  if (m_root) {
    m_root->update(deltaTime);
  }

  for (auto &modal : m_modalStack) {
    modal->update(deltaTime);
  }
}

void UIManager::render(renderer::IRenderer &renderer) {
  if (m_root) {
    m_root->render(renderer);
  }

  for (auto &modal : m_modalStack) {
    modal->render(renderer);
  }
}

void UIManager::handleMouseMove(f32 x, f32 y) {
  f32 deltaX = x - m_mouseX;
  f32 deltaY = y - m_mouseY;
  m_mouseX = x;
  m_mouseY = y;

  Widget *newHovered = hitTest(x, y);

  if (newHovered != m_hoveredWidget) {
    if (m_hoveredWidget) {
      UIEvent leaveEvent;
      leaveEvent.type = UIEventType::MouseLeave;
      leaveEvent.mouseX = x;
      leaveEvent.mouseY = y;
      m_hoveredWidget->handleEvent(leaveEvent);
    }

    m_hoveredWidget = newHovered;

    if (m_hoveredWidget) {
      UIEvent enterEvent;
      enterEvent.type = UIEventType::MouseEnter;
      enterEvent.mouseX = x;
      enterEvent.mouseY = y;
      m_hoveredWidget->handleEvent(enterEvent);
    }
  }

  UIEvent moveEvent;
  moveEvent.type = UIEventType::MouseMove;
  moveEvent.mouseX = x;
  moveEvent.mouseY = y;
  moveEvent.deltaX = deltaX;
  moveEvent.deltaY = deltaY;
  moveEvent.shift = m_shiftDown;
  moveEvent.ctrl = m_ctrlDown;
  moveEvent.alt = m_altDown;

  if (m_pressedWidget) {
    m_pressedWidget->handleEvent(moveEvent);
  } else if (m_hoveredWidget) {
    m_hoveredWidget->handleEvent(moveEvent);
  }
}

void UIManager::handleMouseDown(MouseButton button, f32 x, f32 y) {
  m_mouseDown[static_cast<int>(button)] = true;

  Widget *target = hitTest(x, y);

  UIEvent event;
  event.type = UIEventType::MouseDown;
  event.mouseX = x;
  event.mouseY = y;
  event.button = button;
  event.shift = m_shiftDown;
  event.ctrl = m_ctrlDown;
  event.alt = m_altDown;

  if (target) {
    m_pressedWidget = target;
    target->handleEvent(event);

    if (target->isFocusable()) {
      setFocus(target);
    }
  } else {
    clearFocus();
  }
}

void UIManager::handleMouseUp(MouseButton button, f32 x, f32 y) {
  m_mouseDown[static_cast<int>(button)] = false;

  UIEvent upEvent;
  upEvent.type = UIEventType::MouseUp;
  upEvent.mouseX = x;
  upEvent.mouseY = y;
  upEvent.button = button;
  upEvent.shift = m_shiftDown;
  upEvent.ctrl = m_ctrlDown;
  upEvent.alt = m_altDown;

  Widget *target = m_pressedWidget ? m_pressedWidget : hitTest(x, y);
  if (target) {
    target->handleEvent(upEvent);

    // Generate click event if released on same widget
    if (m_pressedWidget && m_pressedWidget == hitTest(x, y)) {
      UIEvent clickEvent;
      clickEvent.type = UIEventType::Click;
      clickEvent.mouseX = x;
      clickEvent.mouseY = y;
      clickEvent.button = button;
      clickEvent.shift = m_shiftDown;
      clickEvent.ctrl = m_ctrlDown;
      clickEvent.alt = m_altDown;
      m_pressedWidget->handleEvent(clickEvent);
    }
  }

  m_pressedWidget = nullptr;
}

void UIManager::handleMouseScroll(f32 deltaX, f32 deltaY) {
  UIEvent event;
  event.type = UIEventType::Scroll;
  event.mouseX = m_mouseX;
  event.mouseY = m_mouseY;
  event.deltaX = deltaX;
  event.deltaY = deltaY;
  event.shift = m_shiftDown;
  event.ctrl = m_ctrlDown;
  event.alt = m_altDown;

  Widget *target = hitTest(m_mouseX, m_mouseY);
  if (target) {
    target->handleEvent(event);
  }
}

void UIManager::handleKeyDown(i32 keyCode) {
  // Update modifier state
  if (keyCode == 16)
    m_shiftDown = true;
  if (keyCode == 17)
    m_ctrlDown = true;
  if (keyCode == 18)
    m_altDown = true;

  // Tab for focus navigation
  if (keyCode == 9) {
    if (m_shiftDown) {
      focusPrevious();
    } else {
      focusNext();
    }
    return;
  }

  UIEvent event;
  event.type = UIEventType::KeyDown;
  event.keyCode = keyCode;
  event.shift = m_shiftDown;
  event.ctrl = m_ctrlDown;
  event.alt = m_altDown;

  if (m_focusedWidget) {
    m_focusedWidget->handleEvent(event);
  }
}

void UIManager::handleKeyUp(i32 keyCode) {
  if (keyCode == 16)
    m_shiftDown = false;
  if (keyCode == 17)
    m_ctrlDown = false;
  if (keyCode == 18)
    m_altDown = false;

  UIEvent event;
  event.type = UIEventType::KeyUp;
  event.keyCode = keyCode;
  event.shift = m_shiftDown;
  event.ctrl = m_ctrlDown;
  event.alt = m_altDown;

  if (m_focusedWidget) {
    m_focusedWidget->handleEvent(event);
  }
}

void UIManager::handleTextInput(char character) {
  UIEvent event;
  event.type = UIEventType::KeyPress;
  event.character = character;
  event.shift = m_shiftDown;
  event.ctrl = m_ctrlDown;
  event.alt = m_altDown;

  if (m_focusedWidget) {
    m_focusedWidget->handleEvent(event);
  }
}

Widget *UIManager::hitTest(f32 x, f32 y) {
  // Check modals first (in reverse order)
  for (auto it = m_modalStack.rbegin(); it != m_modalStack.rend(); ++it) {
    if (Widget *hit = hitTestRecursive(it->get(), x, y)) {
      return hit;
    }
  }

  // Then check root
  if (m_root) {
    return hitTestRecursive(m_root.get(), x, y);
  }

  return nullptr;
}

void UIManager::invalidateLayout() { m_layoutDirty = true; }

void UIManager::performLayout() {
  if (m_root) {
    m_root->layout();
  }

  for (auto &modal : m_modalStack) {
    modal->layout();
  }

  m_layoutDirty = false;
}

Widget *UIManager::hitTestRecursive(Widget *widget, f32 x, f32 y) {
  if (!widget->isVisible()) {
    return nullptr;
  }

  // Check if point is inside widget bounds
  if (!widget->getBounds().contains(x, y)) {
    return nullptr;
  }

  // Check children in reverse order (top-most first)
  if (auto *container = dynamic_cast<Container *>(widget)) {
    const auto &children = container->getChildren();
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
      if (Widget *hit = hitTestRecursive(it->get(), x, y)) {
        return hit;
      }
    }
  }

  return widget;
}

void UIManager::collectFocusableWidgets(Widget *widget,
                                        std::vector<Widget *> &out) {
  if (!widget->isVisible() || !widget->isEnabled()) {
    return;
  }

  if (widget->isFocusable()) {
    out.push_back(widget);
  }

  if (auto *container = dynamic_cast<Container *>(widget)) {
    for (const auto &child : container->getChildren()) {
      collectFocusableWidgets(child.get(), out);
    }
  }
}

} // namespace NovelMind::ui
