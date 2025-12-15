#include "NovelMind/save/save_manager.hpp"
#include "NovelMind/core/logger.hpp"
#include <chrono>
#include <fstream>

namespace NovelMind::save {

SaveManager::SaveManager() : m_savePath("./saves/") {}

SaveManager::~SaveManager() = default;

Result<void> SaveManager::save(i32 slot, const SaveData &data) {
  if (slot < 0 || slot >= MAX_SLOTS) {
    return Result<void>::error("Invalid save slot");
  }

  std::string filename = getSlotFilename(slot);
  std::ofstream file(filename, std::ios::binary);

  if (!file.is_open()) {
    return Result<void>::error("Failed to open save file: " + filename);
  }

  // Simple binary serialization
  // In production, use a proper serialization library

  auto writeString = [&file](const std::string &str) {
    u32 len = static_cast<u32>(str.size());
    file.write(reinterpret_cast<const char *>(&len), sizeof(len));
    file.write(str.data(), static_cast<std::streamsize>(len));
  };

  // Magic and version
  u32 magic = 0x564D4E53; // "SNMV"
  u16 version = 1;
  file.write(reinterpret_cast<const char *>(&magic), sizeof(magic));
  file.write(reinterpret_cast<const char *>(&version), sizeof(version));

  // Scene and node
  writeString(data.sceneId);
  writeString(data.nodeId);

  // Int variables
  u32 intCount = static_cast<u32>(data.intVariables.size());
  file.write(reinterpret_cast<const char *>(&intCount), sizeof(intCount));
  for (const auto &[name, value] : data.intVariables) {
    writeString(name);
    file.write(reinterpret_cast<const char *>(&value), sizeof(value));
  }

  // Flags
  u32 flagCount = static_cast<u32>(data.flags.size());
  file.write(reinterpret_cast<const char *>(&flagCount), sizeof(flagCount));
  for (const auto &[name, value] : data.flags) {
    writeString(name);
    u8 bval = value ? 1 : 0;
    file.write(reinterpret_cast<const char *>(&bval), sizeof(bval));
  }

  // String variables
  u32 strCount = static_cast<u32>(data.stringVariables.size());
  file.write(reinterpret_cast<const char *>(&strCount), sizeof(strCount));
  for (const auto &[name, value] : data.stringVariables) {
    writeString(name);
    writeString(value);
  }

  // Timestamp
  u64 timestamp = static_cast<u64>(
      std::chrono::system_clock::now().time_since_epoch().count());
  file.write(reinterpret_cast<const char *>(&timestamp), sizeof(timestamp));

  // Checksum
  u32 checksum = calculateChecksum(data);
  file.write(reinterpret_cast<const char *>(&checksum), sizeof(checksum));

  NOVELMIND_LOG_INFO("Saved to slot " + std::to_string(slot));
  return Result<void>::ok();
}

Result<SaveData> SaveManager::load(i32 slot) {
  if (slot < 0 || slot >= MAX_SLOTS) {
    return Result<SaveData>::error("Invalid save slot");
  }

  std::string filename = getSlotFilename(slot);
  std::ifstream file(filename, std::ios::binary);

  if (!file.is_open()) {
    return Result<SaveData>::error("Save file not found: " + filename);
  }

  auto readString = [&file]() -> std::string {
    u32 len;
    file.read(reinterpret_cast<char *>(&len), sizeof(len));
    std::string str(len, '\0');
    file.read(str.data(), static_cast<std::streamsize>(len));
    return str;
  };

  SaveData data;

  // Magic and version
  u32 magic;
  u16 version;
  file.read(reinterpret_cast<char *>(&magic), sizeof(magic));
  file.read(reinterpret_cast<char *>(&version), sizeof(version));

  if (magic != 0x564D4E53) {
    return Result<SaveData>::error("Invalid save file format");
  }

  // Scene and node
  data.sceneId = readString();
  data.nodeId = readString();

  // Int variables
  u32 intCount;
  file.read(reinterpret_cast<char *>(&intCount), sizeof(intCount));
  for (u32 i = 0; i < intCount; ++i) {
    std::string name = readString();
    i32 value;
    file.read(reinterpret_cast<char *>(&value), sizeof(value));
    data.intVariables[name] = value;
  }

  // Flags
  u32 flagCount;
  file.read(reinterpret_cast<char *>(&flagCount), sizeof(flagCount));
  for (u32 i = 0; i < flagCount; ++i) {
    std::string name = readString();
    u8 bval;
    file.read(reinterpret_cast<char *>(&bval), sizeof(bval));
    data.flags[name] = bval != 0;
  }

  // String variables
  u32 strCount;
  file.read(reinterpret_cast<char *>(&strCount), sizeof(strCount));
  for (u32 i = 0; i < strCount; ++i) {
    std::string name = readString();
    std::string value = readString();
    data.stringVariables[name] = value;
  }

  // Timestamp
  file.read(reinterpret_cast<char *>(&data.timestamp), sizeof(data.timestamp));

  // Checksum
  file.read(reinterpret_cast<char *>(&data.checksum), sizeof(data.checksum));

  NOVELMIND_LOG_INFO("Loaded from slot " + std::to_string(slot));
  return Result<SaveData>::ok(std::move(data));
}

Result<void> SaveManager::deleteSave(i32 slot) {
  if (slot < 0 || slot >= MAX_SLOTS) {
    return Result<void>::error("Invalid save slot");
  }

  std::string filename = getSlotFilename(slot);
  if (std::remove(filename.c_str()) != 0) {
    return Result<void>::error("Failed to delete save file");
  }

  NOVELMIND_LOG_INFO("Deleted save slot " + std::to_string(slot));
  return Result<void>::ok();
}

bool SaveManager::slotExists(i32 slot) const {
  if (slot < 0 || slot >= MAX_SLOTS) {
    return false;
  }

  std::ifstream file(getSlotFilename(slot));
  return file.good();
}

std::optional<u64> SaveManager::getSlotTimestamp(i32 /*slot*/) const {
  // Timestamp extraction requires parsing the save file header.
  // For efficiency, consider adding a fixed-position timestamp field
  // at the start of save files in future versions.
  return std::nullopt;
}

i32 SaveManager::getMaxSlots() const { return MAX_SLOTS; }

void SaveManager::setSavePath(const std::string &path) {
  m_savePath = path;
  if (!m_savePath.empty() && m_savePath.back() != '/') {
    m_savePath += '/';
  }
}

const std::string &SaveManager::getSavePath() const { return m_savePath; }

std::string SaveManager::getSlotFilename(i32 slot) const {
  return m_savePath + "save_" + std::to_string(slot) + ".nmsav";
}

u32 SaveManager::calculateChecksum(const SaveData &data) {
  u32 checksum = 0;

  auto hashString = [&checksum](const std::string &str) {
    for (char c : str) {
      checksum = checksum * 31 + static_cast<u32>(c);
    }
  };

  hashString(data.sceneId);
  hashString(data.nodeId);

  for (const auto &[name, value] : data.intVariables) {
    hashString(name);
    checksum = checksum * 31 + static_cast<u32>(value);
  }

  for (const auto &[name, value] : data.flags) {
    hashString(name);
    checksum = checksum * 31 + (value ? 1 : 0);
  }

  for (const auto &[name, value] : data.stringVariables) {
    hashString(name);
    hashString(value);
  }

  return checksum;
}

} // namespace NovelMind::save
