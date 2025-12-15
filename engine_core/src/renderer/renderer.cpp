#include "NovelMind/renderer/renderer.hpp"
#include "NovelMind/core/logger.hpp"

namespace NovelMind::renderer {

class NullRenderer : public IRenderer {
public:
  Result<void> initialize(platform::IWindow &window) override {
    m_width = window.getWidth();
    m_height = window.getHeight();
    NOVELMIND_LOG_WARN("Using null renderer");
    return Result<void>::ok();
  }

  void shutdown() override {
    // Nothing to do
  }

  void beginFrame() override {
    // Nothing to do
  }

  void endFrame() override {
    // Nothing to do
  }

  void clear(const Color & /*color*/) override {
    // Nothing to do
  }

  void setBlendMode(BlendMode /*mode*/) override {
    // Nothing to do
  }

  void drawSprite(const Texture & /*texture*/,
                  const Transform2D & /*transform*/,
                  const Color & /*tint*/) override {
    // Nothing to do
  }

  void drawSprite(const Texture & /*texture*/, const Rect & /*sourceRect*/,
                  const Transform2D & /*transform*/,
                  const Color & /*tint*/) override {
    // Nothing to do
  }

  void drawRect(const Rect & /*rect*/, const Color & /*color*/) override {
    // Nothing to do
  }

  void fillRect(const Rect & /*rect*/, const Color & /*color*/) override {
    // Nothing to do
  }

  void setFade(f32 /*alpha*/, const Color & /*color*/) override {
    // Nothing to do
  }

  [[nodiscard]] i32 getWidth() const override { return m_width; }

  [[nodiscard]] i32 getHeight() const override { return m_height; }

private:
  i32 m_width = 0;
  i32 m_height = 0;
};

std::unique_ptr<IRenderer> createRenderer() {
  // Factory function returns NullRenderer by default.
  // Platform-specific implementations (SDL/OpenGL/Vulkan) are
  // instantiated through platform layer configuration.
  return std::make_unique<NullRenderer>();
}

} // namespace NovelMind::renderer
