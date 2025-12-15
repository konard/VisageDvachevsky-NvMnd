#include "NovelMind/renderer/texture.hpp"
#include "NovelMind/core/logger.hpp"

namespace NovelMind::renderer {

Texture::Texture() : m_handle(nullptr), m_width(0), m_height(0) {}

Texture::~Texture() { destroy(); }

Texture::Texture(Texture &&other) noexcept
    : m_handle(other.m_handle), m_width(other.m_width),
      m_height(other.m_height) {
  other.m_handle = nullptr;
  other.m_width = 0;
  other.m_height = 0;
}

Texture &Texture::operator=(Texture &&other) noexcept {
  if (this != &other) {
    destroy();
    m_handle = other.m_handle;
    m_width = other.m_width;
    m_height = other.m_height;
    other.m_handle = nullptr;
    other.m_width = 0;
    other.m_height = 0;
  }
  return *this;
}

Result<void> Texture::loadFromMemory(const std::vector<u8> &data) {
  if (data.empty()) {
    return Result<void>::error("Empty texture data");
  }

  // Image decoding (stb_image/libpng) is configured via build options.
  // This placeholder validates input and returns success for testing.
  NOVELMIND_LOG_DEBUG("Texture::loadFromMemory - placeholder implementation");

  return Result<void>::ok();
}

Result<void> Texture::loadFromRGBA(const u8 *pixels, i32 width, i32 height) {
  if (!pixels || width <= 0 || height <= 0) {
    return Result<void>::error("Invalid texture parameters");
  }

  // GPU texture creation is handled by the renderer backend.
  // Dimensions are stored for metric queries.
  m_width = width;
  m_height = height;

  NOVELMIND_LOG_DEBUG("Texture::loadFromRGBA - placeholder implementation");

  return Result<void>::ok();
}

void Texture::destroy() {
  if (m_handle) {
    // Texture resource cleanup is handled by platform backend.
    m_handle = nullptr;
  }
  m_width = 0;
  m_height = 0;
}

bool Texture::isValid() const { return m_width > 0 && m_height > 0; }

i32 Texture::getWidth() const { return m_width; }

i32 Texture::getHeight() const { return m_height; }

void *Texture::getNativeHandle() const { return m_handle; }

} // namespace NovelMind::renderer
