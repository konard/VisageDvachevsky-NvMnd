#include "NovelMind/renderer/font.hpp"
#include "NovelMind/core/logger.hpp"

namespace NovelMind::renderer {

Font::Font() : m_handle(nullptr), m_size(0) {}

Font::~Font() { destroy(); }

Font::Font(Font &&other) noexcept
    : m_handle(other.m_handle), m_size(other.m_size) {
  other.m_handle = nullptr;
  other.m_size = 0;
}

Font &Font::operator=(Font &&other) noexcept {
  if (this != &other) {
    destroy();
    m_handle = other.m_handle;
    m_size = other.m_size;
    other.m_handle = nullptr;
    other.m_size = 0;
  }
  return *this;
}

Result<void> Font::loadFromMemory(const std::vector<u8> &data, i32 size) {
  if (data.empty() || size <= 0) {
    return Result<void>::error("Invalid font data or size");
  }

  // Font loading via FreeType is configured through the build system.
  // This placeholder stores the size for metric calculations.
  m_size = size;
  NOVELMIND_LOG_DEBUG("Font::loadFromMemory - placeholder implementation");

  return Result<void>::ok();
}

void Font::destroy() {
  if (m_handle) {
    // Font resource cleanup is handled by platform backend.
    m_handle = nullptr;
  }
  m_size = 0;
}

bool Font::isValid() const { return m_size > 0; }

i32 Font::getSize() const { return m_size; }

void *Font::getNativeHandle() const { return m_handle; }

} // namespace NovelMind::renderer
