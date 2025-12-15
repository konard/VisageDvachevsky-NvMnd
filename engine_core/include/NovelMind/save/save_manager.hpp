#pragma once

#include "NovelMind/core/result.hpp"
#include "NovelMind/core/types.hpp"
#include <map>
#include <optional>
#include <string>

namespace NovelMind::save {

struct SaveData {
  std::string sceneId;
  std::string nodeId;
  std::map<std::string, i32> intVariables;
  std::map<std::string, bool> flags;
  std::map<std::string, std::string> stringVariables;
  u64 timestamp;
  u32 checksum;
};

class SaveManager {
public:
  SaveManager();
  ~SaveManager();

  Result<void> save(i32 slot, const SaveData &data);
  Result<SaveData> load(i32 slot);
  Result<void> deleteSave(i32 slot);

  [[nodiscard]] bool slotExists(i32 slot) const;
  [[nodiscard]] std::optional<u64> getSlotTimestamp(i32 slot) const;

  [[nodiscard]] i32 getMaxSlots() const;

  void setSavePath(const std::string &path);
  [[nodiscard]] const std::string &getSavePath() const;

private:
  [[nodiscard]] std::string getSlotFilename(i32 slot) const;
  [[nodiscard]] static u32 calculateChecksum(const SaveData &data);

  std::string m_savePath;
  static constexpr i32 MAX_SLOTS = 100;
};

} // namespace NovelMind::save
