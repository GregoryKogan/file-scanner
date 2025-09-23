#ifndef SRC_SCANNER_LIB_MD5_FILE_HASHER_H_
#define SRC_SCANNER_LIB_MD5_FILE_HASHER_H_

#include <filesystem>
#include <string>

#include "scanner/interfaces.h"

namespace scanner {

/**
 * @class Md5FileHasher
 * @brief An implementation of IFileHasher that calculates MD5 hashes.
 *
 * This class uses a streaming approach to handle files of any size without
 * consuming large amounts of memory. It is an internal, non-exported class.
 */
class Md5FileHasher final : public IFileHasher {
public:
  /**
   * @brief Calculates the MD5 hash of a given file.
   *
   * Opens the file and reads it in chunks, feeding them into the MD5
   * algorithm.
   *
   * @param file_path The path to the file to be hashed.
   * @return A string representing the lowercase hexadecimal MD5 hash.
   * @throws std::runtime_error if the file cannot be opened.
   * @throws std::ios_base::failure on stream reading errors.
   */
  std::string HashFile(const std::filesystem::path& file_path) override;
};

}  // namespace scanner

#endif  // SRC_SCANNER_LIB_MD5_FILE_HASHER_H_