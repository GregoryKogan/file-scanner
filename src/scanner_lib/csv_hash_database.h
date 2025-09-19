#ifndef SRC_SCANNER_LIB_CSV_HASH_DATABASE_H_
#define SRC_SCANNER_LIB_CSV_HASH_DATABASE_H_

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

#include "scanner/interfaces.h"

namespace scanner {

/**
 * @class CsvHashDatabase
 * @brief An implementation of IHashDatabase that loads signatures from a CSV
 * file.
 *
 * This class parses a semicolon-separated CSV file where each line contains
 * a hash and its corresponding verdict. It stores these signatures in an
 * unordered_map for efficient lookups. This class is an internal, non-exported
 * component of the scanner library.
 */
class CsvHashDatabase final : public IHashDatabase {
public:
  /**
   * @brief Loads malicious signatures from a specified CSV file.
   *
   * Clears any existing data and reads the file line by line. Malformed lines
   * are skipped, and a warning is printed to stderr.
   *
   * @param source_path The path to the CSV database file.
   * @return The total number of signatures successfully loaded.
   * @throws std::runtime_error if the file cannot be opened.
   */
  std::size_t Load(const std::filesystem::path& source_path) override;

  /**
   * @brief Looks up a hash in the loaded database.
   *
   * @param hash The hash string to look up.
   * @return An optional containing the verdict if the hash is found, otherwise
   * std::nullopt.
   */
  std::optional<std::string> FindHash(const std::string& hash) const override;

private:
  std::unordered_map<std::string, std::string> signatures_;
};

}  // namespace scanner

#endif  // SRC_SCANNER_LIB_CSV_HASH_DATABASE_H_