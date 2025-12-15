#pragma once

#include "NovelMind/core/result.hpp"
#include "NovelMind/core/types.hpp"
#include <array>
#include <string>
#include <vector>

namespace NovelMind::VFS {

enum class PackVerificationResult {
  Valid,
  InvalidMagic,
  InvalidVersion,
  CorruptedHeader,
  CorruptedResourceTable,
  CorruptedData,
  ChecksumMismatch,
  SignatureInvalid,
  DecryptionFailed
};

struct PackVerificationReport {
  PackVerificationResult result = PackVerificationResult::Valid;
  std::string message;
  u32 errorOffset = 0;
  std::string resourceId;
};

class PackIntegrityChecker {
public:
  PackIntegrityChecker() = default;

  [[nodiscard]] Result<PackVerificationReport> verifyHeader(const u8 *data,
                                                            usize size);

  [[nodiscard]] Result<PackVerificationReport>
  verifyResourceTable(const u8 *data, usize size, u64 tableOffset,
                      u32 resourceCount);

  [[nodiscard]] Result<PackVerificationReport>
  verifyResource(const u8 *data, usize size, u64 offset, usize resourceSize,
                 u32 expectedChecksum);

  [[nodiscard]] Result<PackVerificationReport>
  verifyPackSignature(const u8 *data, usize size, const u8 *signature,
                      usize signatureSize);

  [[nodiscard]] static u32 calculateCrc32(const u8 *data, usize size);
  [[nodiscard]] static std::array<u8, 32> calculateSha256(const u8 *data,
                                                          usize size);
};

class PackDecryptor {
public:
  PackDecryptor() = default;

  void setKey(const std::vector<u8> &key);
  void setKey(const u8 *key, usize keySize);

  [[nodiscard]] Result<std::vector<u8>> decrypt(const u8 *data, usize size,
                                                const u8 *iv, usize ivSize);

  [[nodiscard]] static std::vector<u8>
  deriveKey(const std::string &password, const u8 *salt, usize saltSize);

  [[nodiscard]] static std::vector<u8> generateRandomIV(usize size = 16);

private:
  std::vector<u8> m_key;
};

class SecurePackReader {
public:
  SecurePackReader() = default;
  ~SecurePackReader() = default;

  void setDecryptor(std::unique_ptr<PackDecryptor> decryptor);
  void setIntegrityChecker(std::unique_ptr<PackIntegrityChecker> checker);

  [[nodiscard]] Result<void> openPack(const std::string &path);
  void closePack();

  [[nodiscard]] Result<std::vector<u8>>
  readResource(const std::string &resourceId);

  [[nodiscard]] bool isOpen() const { return m_isOpen; }
  [[nodiscard]] PackVerificationResult lastVerificationResult() const {
    return m_lastResult;
  }

private:
  std::unique_ptr<PackDecryptor> m_decryptor;
  std::unique_ptr<PackIntegrityChecker> m_integrityChecker;
  bool m_isOpen = false;
  PackVerificationResult m_lastResult = PackVerificationResult::Valid;
};

} // namespace NovelMind::VFS
