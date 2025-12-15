#pragma once

#include "NovelMind/core/result.hpp"
#include "NovelMind/core/types.hpp"
#include <string>
#include <vector>

namespace NovelMind::renderer {

class Font {
public:
  Font();
  ~Font();

  Font(const Font &) = delete;
  Font &operator=(const Font &) = delete;
  Font(Font &&other) noexcept;
  Font &operator=(Font &&other) noexcept;

  Result<void> loadFromMemory(const std::vector<u8> &data, i32 size);
  void destroy();

  [[nodiscard]] bool isValid() const;
  [[nodiscard]] i32 getSize() const;
  [[nodiscard]] void *getNativeHandle() const;

private:
  void *m_handle;
  i32 m_size;
};

} // namespace NovelMind::renderer
