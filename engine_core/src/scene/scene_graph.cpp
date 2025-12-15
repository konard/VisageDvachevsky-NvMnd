#include "NovelMind/scene/scene_graph.hpp"
#include <algorithm>
#include <sstream>

namespace NovelMind::scene {

// ============================================================================
// SceneObjectBase Implementation
// ============================================================================

SceneObjectBase::SceneObjectBase(const std::string &id, SceneObjectType type)
    : m_id(id), m_type(type) {
  m_transform.x = 0.0f;
  m_transform.y = 0.0f;
  m_transform.scaleX = 1.0f;
  m_transform.scaleY = 1.0f;
  m_transform.rotation = 0.0f;
}

const char *SceneObjectBase::getTypeName() const {
  switch (m_type) {
  case SceneObjectType::Base:
    return "Base";
  case SceneObjectType::Background:
    return "Background";
  case SceneObjectType::Character:
    return "Character";
  case SceneObjectType::DialogueUI:
    return "DialogueUI";
  case SceneObjectType::ChoiceUI:
    return "ChoiceUI";
  case SceneObjectType::EffectOverlay:
    return "EffectOverlay";
  case SceneObjectType::Sprite:
    return "Sprite";
  case SceneObjectType::TextLabel:
    return "TextLabel";
  case SceneObjectType::Panel:
    return "Panel";
  case SceneObjectType::Custom:
    return "Custom";
  }
  return "Unknown";
}

void SceneObjectBase::setPosition(f32 x, f32 y) {
  std::string oldX = std::to_string(m_transform.x);
  std::string oldY = std::to_string(m_transform.y);
  m_transform.x = x;
  m_transform.y = y;
  notifyPropertyChanged("x", oldX, std::to_string(x));
  notifyPropertyChanged("y", oldY, std::to_string(y));
}

void SceneObjectBase::setScale(f32 scaleX, f32 scaleY) {
  std::string oldScaleX = std::to_string(m_transform.scaleX);
  std::string oldScaleY = std::to_string(m_transform.scaleY);
  m_transform.scaleX = scaleX;
  m_transform.scaleY = scaleY;
  notifyPropertyChanged("scaleX", oldScaleX, std::to_string(scaleX));
  notifyPropertyChanged("scaleY", oldScaleY, std::to_string(scaleY));
}

void SceneObjectBase::setUniformScale(f32 scale) { setScale(scale, scale); }

void SceneObjectBase::setRotation(f32 angle) {
  std::string oldValue = std::to_string(m_transform.rotation);
  m_transform.rotation = angle;
  notifyPropertyChanged("rotation", oldValue, std::to_string(angle));
}

void SceneObjectBase::setAnchor(f32 anchorX, f32 anchorY) {
  m_anchorX = anchorX;
  m_anchorY = anchorY;
}

void SceneObjectBase::setVisible(bool visible) {
  std::string oldValue = m_visible ? "true" : "false";
  m_visible = visible;
  notifyPropertyChanged("visible", oldValue, visible ? "true" : "false");
}

void SceneObjectBase::setAlpha(f32 alpha) {
  std::string oldValue = std::to_string(m_alpha);
  m_alpha = std::max(0.0f, std::min(1.0f, alpha));
  notifyPropertyChanged("alpha", oldValue, std::to_string(m_alpha));
}

void SceneObjectBase::setZOrder(i32 zOrder) {
  std::string oldValue = std::to_string(m_zOrder);
  m_zOrder = zOrder;
  notifyPropertyChanged("zOrder", oldValue, std::to_string(zOrder));
}

void SceneObjectBase::setParent(SceneObjectBase *parent) { m_parent = parent; }

void SceneObjectBase::addChild(std::unique_ptr<SceneObjectBase> child) {
  if (child) {
    child->setParent(this);
    m_children.push_back(std::move(child));
  }
}

std::unique_ptr<SceneObjectBase>
SceneObjectBase::removeChild(const std::string &id) {
  auto it =
      std::find_if(m_children.begin(), m_children.end(),
                   [&id](const auto &child) { return child->getId() == id; });

  if (it != m_children.end()) {
    auto child = std::move(*it);
    child->setParent(nullptr);
    m_children.erase(it);
    return child;
  }
  return nullptr;
}

SceneObjectBase *SceneObjectBase::findChild(const std::string &id) {
  for (auto &child : m_children) {
    if (child->getId() == id) {
      return child.get();
    }
    if (auto *found = child->findChild(id)) {
      return found;
    }
  }
  return nullptr;
}

void SceneObjectBase::addTag(const std::string &tag) {
  if (std::find(m_tags.begin(), m_tags.end(), tag) == m_tags.end()) {
    m_tags.push_back(tag);
  }
}

void SceneObjectBase::removeTag(const std::string &tag) {
  auto it = std::find(m_tags.begin(), m_tags.end(), tag);
  if (it != m_tags.end()) {
    m_tags.erase(it);
  }
}

bool SceneObjectBase::hasTag(const std::string &tag) const {
  return std::find(m_tags.begin(), m_tags.end(), tag) != m_tags.end();
}

void SceneObjectBase::setProperty(const std::string &name,
                                  const std::string &value) {
  auto it = m_properties.find(name);
  std::string oldValue = (it != m_properties.end()) ? it->second : "";
  m_properties[name] = value;
  notifyPropertyChanged(name, oldValue, value);
}

std::optional<std::string>
SceneObjectBase::getProperty(const std::string &name) const {
  auto it = m_properties.find(name);
  if (it != m_properties.end()) {
    return it->second;
  }
  return std::nullopt;
}

void SceneObjectBase::update(f64 deltaTime) {
  // Update animations
  for (auto it = m_animations.begin(); it != m_animations.end();) {
    if (*it && !(*it)->update(deltaTime)) {
      it = m_animations.erase(it);
    } else {
      ++it;
    }
  }

  // Update children
  for (auto &child : m_children) {
    child->update(deltaTime);
  }
}

SceneObjectState SceneObjectBase::saveState() const {
  SceneObjectState state;
  state.id = m_id;
  state.type = m_type;
  state.x = m_transform.x;
  state.y = m_transform.y;
  state.scaleX = m_transform.scaleX;
  state.scaleY = m_transform.scaleY;
  state.rotation = m_transform.rotation;
  state.alpha = m_alpha;
  state.visible = m_visible;
  state.zOrder = m_zOrder;
  state.properties = m_properties;
  return state;
}

void SceneObjectBase::loadState(const SceneObjectState &state) {
  m_transform.x = state.x;
  m_transform.y = state.y;
  m_transform.scaleX = state.scaleX;
  m_transform.scaleY = state.scaleY;
  m_transform.rotation = state.rotation;
  m_alpha = state.alpha;
  m_visible = state.visible;
  m_zOrder = state.zOrder;
  m_properties = state.properties;
}

void SceneObjectBase::animatePosition(f32 toX, f32 toY, f32 duration,
                                      EaseType easing) {
  auto tween = std::make_unique<PositionTween>(&m_transform.x, &m_transform.y,
                                               m_transform.x, m_transform.y,
                                               toX, toY, duration, easing);
  tween->start();
  m_animations.push_back(std::move(tween));
}

void SceneObjectBase::animateAlpha(f32 toAlpha, f32 duration, EaseType easing) {
  auto tween = std::make_unique<FloatTween>(&m_alpha, m_alpha, toAlpha,
                                            duration, easing);
  tween->start();
  m_animations.push_back(std::move(tween));
}

void SceneObjectBase::animateScale(f32 toScaleX, f32 toScaleY, f32 duration,
                                   EaseType easing) {
  auto tweenX = std::make_unique<FloatTween>(
      &m_transform.scaleX, m_transform.scaleX, toScaleX, duration, easing);
  auto tweenY = std::make_unique<FloatTween>(
      &m_transform.scaleY, m_transform.scaleY, toScaleY, duration, easing);
  tweenX->start();
  tweenY->start();
  m_animations.push_back(std::move(tweenX));
  m_animations.push_back(std::move(tweenY));
}

void SceneObjectBase::notifyPropertyChanged(const std::string &property,
                                            const std::string &oldValue,
                                            const std::string &newValue) {
  if (m_observer) {
    PropertyChange change;
    change.objectId = m_id;
    change.propertyName = property;
    change.oldValue = oldValue;
    change.newValue = newValue;
    m_observer->onPropertyChanged(change);
  }
}

// ============================================================================
// BackgroundObject Implementation
// ============================================================================

BackgroundObject::BackgroundObject(const std::string &id)
    : SceneObjectBase(id, SceneObjectType::Background) {}

void BackgroundObject::setTextureId(const std::string &textureId) {
  std::string oldValue = m_textureId;
  m_textureId = textureId;
  notifyPropertyChanged("textureId", oldValue, textureId);
}

void BackgroundObject::setTint(const renderer::Color &color) { m_tint = color; }

void BackgroundObject::render(renderer::IRenderer & /*renderer*/) {
  if (!m_visible || m_alpha <= 0.0f) {
    return;
  }

  // Texture rendering requires ResourceManager integration.
  // The renderer needs a Texture object from the resource manager.
  // When available, the background is drawn using:
  //   auto* texture = resourceManager->getTexture(m_textureId);
  //   renderer.drawSprite(*texture, m_transform, tintWithAlpha);
}

SceneObjectState BackgroundObject::saveState() const {
  auto state = SceneObjectBase::saveState();
  state.properties["textureId"] = m_textureId;
  std::stringstream ss;
  ss << static_cast<int>(m_tint.r) << "," << static_cast<int>(m_tint.g) << ","
     << static_cast<int>(m_tint.b) << "," << static_cast<int>(m_tint.a);
  state.properties["tint"] = ss.str();
  return state;
}

void BackgroundObject::loadState(const SceneObjectState &state) {
  SceneObjectBase::loadState(state);
  auto texIt = state.properties.find("textureId");
  if (texIt != state.properties.end()) {
    m_textureId = texIt->second;
  }
}

// ============================================================================
// CharacterObject Implementation
// ============================================================================

CharacterObject::CharacterObject(const std::string &id,
                                 const std::string &characterId)
    : SceneObjectBase(id, SceneObjectType::Character),
      m_characterId(characterId) {}

void CharacterObject::setCharacterId(const std::string &characterId) {
  m_characterId = characterId;
}

void CharacterObject::setDisplayName(const std::string &name) {
  m_displayName = name;
}

void CharacterObject::setExpression(const std::string &expression) {
  std::string oldValue = m_expression;
  m_expression = expression;
  notifyPropertyChanged("expression", oldValue, expression);
}

void CharacterObject::setPose(const std::string &pose) {
  std::string oldValue = m_pose;
  m_pose = pose;
  notifyPropertyChanged("pose", oldValue, pose);
}

void CharacterObject::setSlotPosition(Position pos) { m_slotPosition = pos; }

void CharacterObject::setNameColor(const renderer::Color &color) {
  m_nameColor = color;
}

void CharacterObject::setHighlighted(bool highlighted) {
  m_highlighted = highlighted;
}

void CharacterObject::render(renderer::IRenderer & /*renderer*/) {
  if (!m_visible || m_alpha <= 0.0f) {
    return;
  }

  // Character rendering requires ResourceManager integration.
  // Texture ID is composed from character, expression, and pose.
  // Non-highlighted characters are rendered with dimmed colors.
}

SceneObjectState CharacterObject::saveState() const {
  auto state = SceneObjectBase::saveState();
  state.properties["characterId"] = m_characterId;
  state.properties["displayName"] = m_displayName;
  state.properties["expression"] = m_expression;
  state.properties["pose"] = m_pose;
  state.properties["slotPosition"] =
      std::to_string(static_cast<int>(m_slotPosition));
  state.properties["highlighted"] = m_highlighted ? "true" : "false";
  return state;
}

void CharacterObject::loadState(const SceneObjectState &state) {
  SceneObjectBase::loadState(state);

  auto it = state.properties.find("characterId");
  if (it != state.properties.end())
    m_characterId = it->second;

  it = state.properties.find("displayName");
  if (it != state.properties.end())
    m_displayName = it->second;

  it = state.properties.find("expression");
  if (it != state.properties.end())
    m_expression = it->second;

  it = state.properties.find("pose");
  if (it != state.properties.end())
    m_pose = it->second;

  it = state.properties.find("slotPosition");
  if (it != state.properties.end()) {
    m_slotPosition = static_cast<Position>(std::stoi(it->second));
  }

  it = state.properties.find("highlighted");
  if (it != state.properties.end()) {
    m_highlighted = (it->second == "true");
  }
}

void CharacterObject::animateToSlot(Position slot, f32 duration,
                                    EaseType easing) {
  // Calculate target position based on slot
  f32 targetX = 0.0f;
  switch (slot) {
  case Position::Left:
    targetX = 200.0f;
    break;
  case Position::Center:
    targetX = 640.0f;
    break;
  case Position::Right:
    targetX = 1080.0f;
    break;
  case Position::Custom:
    return; // Don't animate custom positions
  }

  m_slotPosition = slot;
  animatePosition(targetX, m_transform.y, duration, easing);
}

// ============================================================================
// DialogueUIObject Implementation
// ============================================================================

DialogueUIObject::DialogueUIObject(const std::string &id)
    : SceneObjectBase(id, SceneObjectType::DialogueUI) {}

void DialogueUIObject::setSpeaker(const std::string &speaker) {
  m_speaker = speaker;
}

void DialogueUIObject::setText(const std::string &text) {
  m_text = text;
  m_typewriterProgress = 0.0f;
  m_typewriterComplete = !m_typewriterEnabled;
}

void DialogueUIObject::setSpeakerColor(const renderer::Color &color) {
  m_speakerColor = color;
}

void DialogueUIObject::setBackgroundTextureId(const std::string &textureId) {
  m_backgroundTextureId = textureId;
}

void DialogueUIObject::setTypewriterEnabled(bool enabled) {
  m_typewriterEnabled = enabled;
}

void DialogueUIObject::setTypewriterSpeed(f32 charsPerSecond) {
  m_typewriterSpeed = charsPerSecond;
}

void DialogueUIObject::startTypewriter() {
  m_typewriterProgress = 0.0f;
  m_typewriterComplete = false;
}

void DialogueUIObject::skipTypewriter() {
  m_typewriterProgress = static_cast<f32>(m_text.length());
  m_typewriterComplete = true;
}

void DialogueUIObject::update(f64 deltaTime) {
  SceneObjectBase::update(deltaTime);

  if (m_typewriterEnabled && !m_typewriterComplete) {
    m_typewriterProgress += static_cast<f32>(deltaTime) * m_typewriterSpeed;
    if (m_typewriterProgress >= static_cast<f32>(m_text.length())) {
      m_typewriterProgress = static_cast<f32>(m_text.length());
      m_typewriterComplete = true;
    }
  }
}

void DialogueUIObject::render(renderer::IRenderer & /*renderer*/) {
  if (!m_visible || m_alpha <= 0.0f) {
    return;
  }

  // Dialogue rendering requires ResourceManager and TextRenderer integration.
  // Background texture is rendered first if available.
  // Speaker name is rendered with the configured speaker color.
  // Text is rendered with typewriter effect when enabled.
}

SceneObjectState DialogueUIObject::saveState() const {
  auto state = SceneObjectBase::saveState();
  state.properties["speaker"] = m_speaker;
  state.properties["text"] = m_text;
  state.properties["backgroundTextureId"] = m_backgroundTextureId;
  state.properties["typewriterEnabled"] =
      m_typewriterEnabled ? "true" : "false";
  state.properties["typewriterSpeed"] = std::to_string(m_typewriterSpeed);
  return state;
}

void DialogueUIObject::loadState(const SceneObjectState &state) {
  SceneObjectBase::loadState(state);

  auto it = state.properties.find("speaker");
  if (it != state.properties.end())
    m_speaker = it->second;

  it = state.properties.find("text");
  if (it != state.properties.end())
    m_text = it->second;

  it = state.properties.find("backgroundTextureId");
  if (it != state.properties.end())
    m_backgroundTextureId = it->second;

  it = state.properties.find("typewriterEnabled");
  if (it != state.properties.end()) {
    m_typewriterEnabled = (it->second == "true");
  }

  it = state.properties.find("typewriterSpeed");
  if (it != state.properties.end()) {
    m_typewriterSpeed = std::stof(it->second);
  }
}

// ============================================================================
// ChoiceUIObject Implementation
// ============================================================================

ChoiceUIObject::ChoiceUIObject(const std::string &id)
    : SceneObjectBase(id, SceneObjectType::ChoiceUI) {}

void ChoiceUIObject::setChoices(const std::vector<ChoiceOption> &choices) {
  m_choices = choices;
  m_selectedIndex = 0;
}

void ChoiceUIObject::clearChoices() {
  m_choices.clear();
  m_selectedIndex = 0;
}

void ChoiceUIObject::setSelectedIndex(i32 index) {
  if (index >= 0 && index < static_cast<i32>(m_choices.size())) {
    m_selectedIndex = index;
  }
}

void ChoiceUIObject::selectNext() {
  if (m_choices.empty()) {
    return;
  }

  i32 start = m_selectedIndex;
  do {
    m_selectedIndex =
        (m_selectedIndex + 1) % static_cast<i32>(m_choices.size());
  } while (!m_choices[static_cast<size_t>(m_selectedIndex)].enabled &&
           m_selectedIndex != start);
}

void ChoiceUIObject::selectPrevious() {
  if (m_choices.empty()) {
    return;
  }

  i32 start = m_selectedIndex;
  do {
    m_selectedIndex =
        (m_selectedIndex - 1 + static_cast<i32>(m_choices.size())) %
        static_cast<i32>(m_choices.size());
  } while (!m_choices[static_cast<size_t>(m_selectedIndex)].enabled &&
           m_selectedIndex != start);
}

bool ChoiceUIObject::confirm() {
  if (m_selectedIndex >= 0 &&
      m_selectedIndex < static_cast<i32>(m_choices.size())) {
    const auto &choice = m_choices[static_cast<size_t>(m_selectedIndex)];
    if (choice.enabled && m_onSelect) {
      m_onSelect(m_selectedIndex, choice.id);
      return true;
    }
  }
  return false;
}

void ChoiceUIObject::setOnSelect(
    std::function<void(i32, const std::string &)> callback) {
  m_onSelect = std::move(callback);
}

void ChoiceUIObject::render(renderer::IRenderer & /*renderer*/) {
  if (!m_visible || m_alpha <= 0.0f) {
    return;
  }

  // Choice menu rendering requires TextRenderer integration.
  // Choices are rendered vertically with spacing.
  // Selected choice is highlighted in yellow.
  // Disabled choices are rendered in gray.
}

SceneObjectState ChoiceUIObject::saveState() const {
  auto state = SceneObjectBase::saveState();
  state.properties["choiceCount"] = std::to_string(m_choices.size());
  for (size_t i = 0; i < m_choices.size(); ++i) {
    std::string prefix = "choice" + std::to_string(i) + "_";
    state.properties[prefix + "id"] = m_choices[i].id;
    state.properties[prefix + "text"] = m_choices[i].text;
    state.properties[prefix + "enabled"] =
        m_choices[i].enabled ? "true" : "false";
    state.properties[prefix + "visible"] =
        m_choices[i].visible ? "true" : "false";
  }
  state.properties["selectedIndex"] = std::to_string(m_selectedIndex);
  return state;
}

void ChoiceUIObject::loadState(const SceneObjectState &state) {
  SceneObjectBase::loadState(state);

  m_choices.clear();
  auto countIt = state.properties.find("choiceCount");
  if (countIt != state.properties.end()) {
    size_t count = std::stoul(countIt->second);
    for (size_t i = 0; i < count; ++i) {
      std::string prefix = "choice" + std::to_string(i) + "_";
      ChoiceOption option;

      auto it = state.properties.find(prefix + "id");
      if (it != state.properties.end())
        option.id = it->second;

      it = state.properties.find(prefix + "text");
      if (it != state.properties.end())
        option.text = it->second;

      it = state.properties.find(prefix + "enabled");
      if (it != state.properties.end())
        option.enabled = (it->second == "true");

      it = state.properties.find(prefix + "visible");
      if (it != state.properties.end())
        option.visible = (it->second == "true");

      m_choices.push_back(option);
    }
  }

  auto selIt = state.properties.find("selectedIndex");
  if (selIt != state.properties.end()) {
    m_selectedIndex = std::stoi(selIt->second);
  }
}

// ============================================================================
// EffectOverlayObject Implementation
// ============================================================================

EffectOverlayObject::EffectOverlayObject(const std::string &id)
    : SceneObjectBase(id, SceneObjectType::EffectOverlay) {}

void EffectOverlayObject::setEffectType(EffectType type) {
  m_effectType = type;
}

void EffectOverlayObject::setColor(const renderer::Color &color) {
  m_color = color;
}

void EffectOverlayObject::setIntensity(f32 intensity) {
  m_intensity = std::max(0.0f, std::min(1.0f, intensity));
}

void EffectOverlayObject::startEffect(f32 duration) {
  m_effectActive = true;
  m_effectTimer = 0.0f;
  m_effectDuration = duration;
}

void EffectOverlayObject::stopEffect() {
  m_effectActive = false;
  m_effectTimer = 0.0f;
}

void EffectOverlayObject::update(f64 deltaTime) {
  SceneObjectBase::update(deltaTime);

  if (m_effectActive && m_effectDuration > 0.0f) {
    m_effectTimer += static_cast<f32>(deltaTime);
    if (m_effectTimer >= m_effectDuration) {
      m_effectActive = false;
      m_effectTimer = 0.0f;
    }
  }
}

void EffectOverlayObject::render(renderer::IRenderer &renderer) {
  if (!m_visible || m_alpha <= 0.0f || !m_effectActive) {
    return;
  }

  renderer::Color effectColor = m_color;
  f32 progress =
      m_effectDuration > 0.0f ? m_effectTimer / m_effectDuration : 0.0f;

  switch (m_effectType) {
  case EffectType::Fade: {
    f32 fade = 1.0f - progress;
    effectColor.a = static_cast<u8>(m_color.a * m_alpha * m_intensity * fade);
    renderer::Rect fullscreen{0.0f, 0.0f, 1920.0f, 1080.0f};
    renderer.fillRect(fullscreen, effectColor);
    break;
  }
  case EffectType::Flash: {
    f32 flash = 1.0f - progress * progress;
    effectColor.a = static_cast<u8>(m_color.a * m_alpha * m_intensity * flash);
    renderer::Rect fullscreen{0.0f, 0.0f, 1920.0f, 1080.0f};
    renderer.fillRect(fullscreen, effectColor);
    break;
  }
  case EffectType::Shake:
    // Shake is handled by modifying camera/scene offset, not rendered directly
    break;
  case EffectType::Rain:
  case EffectType::Snow:
    // Particle effects would be rendered here
    break;
  case EffectType::None:
  case EffectType::Custom:
    break;
  }
}

SceneObjectState EffectOverlayObject::saveState() const {
  auto state = SceneObjectBase::saveState();
  state.properties["effectType"] =
      std::to_string(static_cast<int>(m_effectType));
  state.properties["intensity"] = std::to_string(m_intensity);
  state.properties["effectActive"] = m_effectActive ? "true" : "false";
  return state;
}

void EffectOverlayObject::loadState(const SceneObjectState &state) {
  SceneObjectBase::loadState(state);

  auto it = state.properties.find("effectType");
  if (it != state.properties.end()) {
    m_effectType = static_cast<EffectType>(std::stoi(it->second));
  }

  it = state.properties.find("intensity");
  if (it != state.properties.end()) {
    m_intensity = std::stof(it->second);
  }

  it = state.properties.find("effectActive");
  if (it != state.properties.end()) {
    m_effectActive = (it->second == "true");
  }
}

// ============================================================================
// Layer Implementation
// ============================================================================

Layer::Layer(const std::string &name, LayerType type)
    : m_name(name), m_type(type) {}

void Layer::addObject(std::unique_ptr<SceneObjectBase> object) {
  if (object) {
    m_objects.push_back(std::move(object));
    sortByZOrder();
  }
}

std::unique_ptr<SceneObjectBase> Layer::removeObject(const std::string &id) {
  auto it = std::find_if(m_objects.begin(), m_objects.end(),
                         [&id](const auto &obj) { return obj->getId() == id; });

  if (it != m_objects.end()) {
    auto obj = std::move(*it);
    m_objects.erase(it);
    return obj;
  }
  return nullptr;
}

void Layer::clear() { m_objects.clear(); }

SceneObjectBase *Layer::findObject(const std::string &id) {
  auto it = std::find_if(m_objects.begin(), m_objects.end(),
                         [&id](const auto &obj) { return obj->getId() == id; });
  return (it != m_objects.end()) ? it->get() : nullptr;
}

const SceneObjectBase *Layer::findObject(const std::string &id) const {
  auto it = std::find_if(m_objects.begin(), m_objects.end(),
                         [&id](const auto &obj) { return obj->getId() == id; });
  return (it != m_objects.end()) ? it->get() : nullptr;
}

void Layer::setVisible(bool visible) { m_visible = visible; }

void Layer::setAlpha(f32 alpha) {
  m_alpha = std::max(0.0f, std::min(1.0f, alpha));
}

void Layer::sortByZOrder() {
  std::stable_sort(m_objects.begin(), m_objects.end(),
                   [](const auto &a, const auto &b) {
                     return a->getZOrder() < b->getZOrder();
                   });
}

void Layer::update(f64 deltaTime) {
  if (!m_visible) {
    return;
  }

  for (auto &obj : m_objects) {
    obj->update(deltaTime);
  }
}

void Layer::render(renderer::IRenderer &renderer) {
  if (!m_visible || m_alpha <= 0.0f) {
    return;
  }

  for (auto &obj : m_objects) {
    if (obj->isVisible()) {
      obj->render(renderer);
    }
  }
}

// ============================================================================
// SceneGraph Implementation
// ============================================================================

SceneGraph::SceneGraph()
    : m_backgroundLayer("Background", LayerType::Background),
      m_characterLayer("Characters", LayerType::Characters),
      m_uiLayer("UI", LayerType::UI),
      m_effectLayer("Effects", LayerType::Effects) {}

SceneGraph::~SceneGraph() = default;

void SceneGraph::setSceneId(const std::string &id) { m_sceneId = id; }

void SceneGraph::clear() {
  m_backgroundLayer.clear();
  m_characterLayer.clear();
  m_uiLayer.clear();
  m_effectLayer.clear();
  m_objectLookup.clear();
}

Layer &SceneGraph::getLayer(LayerType type) {
  switch (type) {
  case LayerType::Background:
    return m_backgroundLayer;
  case LayerType::Characters:
    return m_characterLayer;
  case LayerType::UI:
    return m_uiLayer;
  case LayerType::Effects:
    return m_effectLayer;
  }
  return m_backgroundLayer; // Default fallback
}

void SceneGraph::addToLayer(LayerType layer,
                            std::unique_ptr<SceneObjectBase> object) {
  if (!object) {
    return;
  }

  std::string id = object->getId();
  SceneObjectType type = object->getType();
  registerObject(object.get());
  getLayer(layer).addObject(std::move(object));
  onObjectAdded(id, type);
}

std::unique_ptr<SceneObjectBase>
SceneGraph::removeFromLayer(LayerType layer, const std::string &id) {
  auto obj = getLayer(layer).removeObject(id);
  if (obj) {
    m_objectLookup.erase(id);
    onObjectRemoved(id);
  }
  return obj;
}

SceneObjectBase *SceneGraph::findObject(const std::string &id) {
  auto it = m_objectLookup.find(id);
  return (it != m_objectLookup.end()) ? it->second : nullptr;
}

std::vector<SceneObjectBase *>
SceneGraph::findObjectsByTag(const std::string &tag) {
  std::vector<SceneObjectBase *> result;
  for (auto &[id, obj] : m_objectLookup) {
    if (obj->hasTag(tag)) {
      result.push_back(obj);
    }
  }
  return result;
}

std::vector<SceneObjectBase *>
SceneGraph::findObjectsByType(SceneObjectType type) {
  std::vector<SceneObjectBase *> result;
  for (auto &[id, obj] : m_objectLookup) {
    if (obj->getType() == type) {
      result.push_back(obj);
    }
  }
  return result;
}

void SceneGraph::showBackground(const std::string &textureId) {
  // Clear existing backgrounds
  m_backgroundLayer.clear();
  m_objectLookup.erase("main_background");

  auto bg = std::make_unique<BackgroundObject>("main_background");
  bg->setTextureId(textureId);
  registerObject(bg.get());
  m_backgroundLayer.addObject(std::move(bg));
  onObjectAdded("main_background", SceneObjectType::Background);
}

CharacterObject *SceneGraph::showCharacter(const std::string &id,
                                           const std::string &characterId,
                                           CharacterObject::Position position) {
  // Check if character already exists
  auto *existing = findObject(id);
  if (existing && existing->getType() == SceneObjectType::Character) {
    auto *character = static_cast<CharacterObject *>(existing);
    character->setSlotPosition(position);
    character->setVisible(true);
    return character;
  }

  // Create new character
  auto character = std::make_unique<CharacterObject>(id, characterId);
  character->setSlotPosition(position);

  // Set initial position based on slot
  f32 x = 640.0f;
  switch (position) {
  case CharacterObject::Position::Left:
    x = 200.0f;
    break;
  case CharacterObject::Position::Center:
    x = 640.0f;
    break;
  case CharacterObject::Position::Right:
    x = 1080.0f;
    break;
  case CharacterObject::Position::Custom:
    break;
  }
  character->setPosition(x, 400.0f);

  CharacterObject *ptr = character.get();
  registerObject(character.get());
  m_characterLayer.addObject(std::move(character));
  onObjectAdded(id, SceneObjectType::Character);

  return ptr;
}

void SceneGraph::hideCharacter(const std::string &id) {
  auto *obj = findObject(id);
  if (obj) {
    obj->setVisible(false);
  }
}

DialogueUIObject *SceneGraph::showDialogue(const std::string &speaker,
                                           const std::string &text) {
  auto *existing = findObject("dialogue_box");
  if (existing && existing->getType() == SceneObjectType::DialogueUI) {
    auto *dialogue = static_cast<DialogueUIObject *>(existing);
    dialogue->setSpeaker(speaker);
    dialogue->setText(text);
    dialogue->setVisible(true);
    dialogue->startTypewriter();
    return dialogue;
  }

  auto dialogue = std::make_unique<DialogueUIObject>("dialogue_box");
  dialogue->setSpeaker(speaker);
  dialogue->setText(text);
  dialogue->setPosition(0.0f, 600.0f);
  dialogue->startTypewriter();

  DialogueUIObject *ptr = dialogue.get();
  registerObject(dialogue.get());
  m_uiLayer.addObject(std::move(dialogue));
  onObjectAdded("dialogue_box", SceneObjectType::DialogueUI);

  return ptr;
}

void SceneGraph::hideDialogue() {
  auto *obj = findObject("dialogue_box");
  if (obj) {
    obj->setVisible(false);
  }
}

ChoiceUIObject *SceneGraph::showChoices(
    const std::vector<ChoiceUIObject::ChoiceOption> &choices) {
  auto *existing = findObject("choice_menu");
  if (existing && existing->getType() == SceneObjectType::ChoiceUI) {
    auto *menu = static_cast<ChoiceUIObject *>(existing);
    menu->setChoices(choices);
    menu->setVisible(true);
    return menu;
  }

  auto menu = std::make_unique<ChoiceUIObject>("choice_menu");
  menu->setChoices(choices);
  menu->setPosition(400.0f, 300.0f);

  ChoiceUIObject *ptr = menu.get();
  registerObject(menu.get());
  m_uiLayer.addObject(std::move(menu));
  onObjectAdded("choice_menu", SceneObjectType::ChoiceUI);

  return ptr;
}

void SceneGraph::hideChoices() {
  auto *obj = findObject("choice_menu");
  if (obj) {
    obj->setVisible(false);
  }
}

void SceneGraph::update(f64 deltaTime) {
  m_backgroundLayer.update(deltaTime);
  m_characterLayer.update(deltaTime);
  m_uiLayer.update(deltaTime);
  m_effectLayer.update(deltaTime);
}

void SceneGraph::render(renderer::IRenderer &renderer) {
  m_backgroundLayer.render(renderer);
  m_characterLayer.render(renderer);
  m_uiLayer.render(renderer);
  m_effectLayer.render(renderer);
}

SceneState SceneGraph::saveState() const {
  SceneState state;
  state.sceneId = m_sceneId;

  // Save all objects from all layers
  auto saveLayerObjects = [&state](const Layer &layer) {
    for (const auto &obj : layer.getObjects()) {
      state.objects.push_back(obj->saveState());
    }
  };

  saveLayerObjects(m_backgroundLayer);
  saveLayerObjects(m_characterLayer);
  saveLayerObjects(m_uiLayer);
  saveLayerObjects(m_effectLayer);

  // Track active background and characters
  auto *bg =
      const_cast<Layer &>(m_backgroundLayer).findObject("main_background");
  if (bg) {
    auto *bgObj = static_cast<const BackgroundObject *>(bg);
    state.activeBackground = bgObj->getTextureId();
  }

  for (const auto &obj : m_characterLayer.getObjects()) {
    if (obj->isVisible()) {
      state.visibleCharacters.push_back(obj->getId());
    }
  }

  return state;
}

void SceneGraph::loadState(const SceneState &state) {
  clear();
  m_sceneId = state.sceneId;

  // Recreate objects from saved state
  for (const auto &objState : state.objects) {
    std::unique_ptr<SceneObjectBase> obj;
    LayerType layer = LayerType::Background;

    switch (objState.type) {
    case SceneObjectType::Background: {
      auto bg = std::make_unique<BackgroundObject>(objState.id);
      bg->loadState(objState);
      obj = std::move(bg);
      layer = LayerType::Background;
      break;
    }
    case SceneObjectType::Character: {
      auto character = std::make_unique<CharacterObject>(objState.id, "");
      character->loadState(objState);
      obj = std::move(character);
      layer = LayerType::Characters;
      break;
    }
    case SceneObjectType::DialogueUI: {
      auto dialogue = std::make_unique<DialogueUIObject>(objState.id);
      dialogue->loadState(objState);
      obj = std::move(dialogue);
      layer = LayerType::UI;
      break;
    }
    case SceneObjectType::ChoiceUI: {
      auto choice = std::make_unique<ChoiceUIObject>(objState.id);
      choice->loadState(objState);
      obj = std::move(choice);
      layer = LayerType::UI;
      break;
    }
    case SceneObjectType::EffectOverlay: {
      auto effect = std::make_unique<EffectOverlayObject>(objState.id);
      effect->loadState(objState);
      obj = std::move(effect);
      layer = LayerType::Effects;
      break;
    }
    default:
      continue;
    }

    if (obj) {
      addToLayer(layer, std::move(obj));
    }
  }
}

void SceneGraph::addObserver(ISceneObserver *observer) {
  if (observer && std::find(m_observers.begin(), m_observers.end(), observer) ==
                      m_observers.end()) {
    m_observers.push_back(observer);
  }
}

void SceneGraph::removeObserver(ISceneObserver *observer) {
  auto it = std::find(m_observers.begin(), m_observers.end(), observer);
  if (it != m_observers.end()) {
    m_observers.erase(it);
  }
}

void SceneGraph::onObjectAdded(const std::string &objectId,
                               SceneObjectType type) {
  notifyObservers(
      [&](ISceneObserver *obs) { obs->onObjectAdded(objectId, type); });
}

void SceneGraph::onObjectRemoved(const std::string &objectId) {
  notifyObservers([&](ISceneObserver *obs) { obs->onObjectRemoved(objectId); });
}

void SceneGraph::onPropertyChanged(const PropertyChange &change) {
  notifyObservers([&](ISceneObserver *obs) { obs->onPropertyChanged(change); });
}

void SceneGraph::onLayerChanged(const std::string &objectId,
                                const std::string &newLayer) {
  notifyObservers(
      [&](ISceneObserver *obs) { obs->onLayerChanged(objectId, newLayer); });
}

void SceneGraph::notifyObservers(
    const std::function<void(ISceneObserver *)> &notify) {
  for (auto *observer : m_observers) {
    if (observer) {
      notify(observer);
    }
  }
}

void SceneGraph::registerObject(SceneObjectBase *obj) {
  if (obj) {
    obj->m_observer = this;
    m_objectLookup[obj->getId()] = obj;
  }
}

} // namespace NovelMind::scene
