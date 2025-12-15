#include "NovelMind/input/input_manager.hpp"

namespace NovelMind::input {

InputManager::InputManager() : m_mouseX(0), m_mouseY(0) {}

InputManager::~InputManager() = default;

void InputManager::update() {
  // Input polling is handled by platform-specific backends.
  // This base implementation serves as a stub for testing.
}

bool InputManager::isKeyDown(Key /*key*/) const {
  // Platform backend provides actual key state.
  return false;
}

bool InputManager::isKeyPressed(Key /*key*/) const {
  // Platform backend provides actual key state.
  return false;
}

bool InputManager::isKeyReleased(Key /*key*/) const {
  // Platform backend provides actual key state.
  return false;
}

bool InputManager::isMouseButtonDown(MouseButton /*button*/) const {
  // Platform backend provides actual mouse state.
  return false;
}

bool InputManager::isMouseButtonPressed(MouseButton /*button*/) const {
  // Platform backend provides actual mouse state.
  return false;
}

bool InputManager::isMouseButtonReleased(MouseButton /*button*/) const {
  // Platform backend provides actual mouse state.
  return false;
}

i32 InputManager::getMouseX() const { return m_mouseX; }

i32 InputManager::getMouseY() const { return m_mouseY; }

} // namespace NovelMind::input
