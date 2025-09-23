#ifndef SCANNER_INTERFACES_H_
#define SCANNER_INTERFACES_H_

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "scanner/domain.h"
#include "scanner/visibility.h"

namespace scanner {

/**
 * @interface IFileHasher
 * @brief Defines the contract for a component that can hash a file's content.
 *
 * This abstraction allows the core scanning logic to be independent of the
 * specific hashing algorithm used (e.g., MD5, SHA256).
 */
class IFileHasher {
public:
  virtual ~IFileHasher() = default;

  /**
   * @brief Calculates the hash of a given file.
   * @param file_path The path to the file to be hashed.
   * @return A string representing the hexadecimal hash of the file.
   * @throws std::runtime_error if the file cannot be opened or read.
   */
  virtual std::string HashFile(const std::filesystem::path& file_path) = 0;
};

/**
 * @interface IHashDatabase
 * @brief Defines the contract for a database of malicious signatures.
 */
class SCANNER_API IHashDatabase {
public:
  virtual ~IHashDatabase() = default;

  /**
   * @brief Loads malicious signatures from a source.
   * @param source_path The path to the data source (e.g., a CSV file).
   * @return The number of signatures successfully loaded.
   * @throws std::runtime_error on failure to open or parse the source.
   */
  virtual size_t Load(const std::filesystem::path& source_path) = 0;

  /**
   * @brief Looks up a hash to see if it is in the database.
   * @param hash The hash string to look up.
   * @return An optional containing the verdict if the hash is found, otherwise
   * std::nullopt.
   */
  virtual std::optional<std::string> FindHash(
      const std::string& hash) const = 0;
};

/**
 * @interface ILogger
 * @brief Defines the contract for a component that logs malicious detections.
 *
 * Implementations of this interface must be thread-safe, as multiple scanning
 * threads may report detections concurrently.
 */
class SCANNER_API ILogger {
public:
  virtual ~ILogger() = default;

  /**
   * @brief Logs the detection of a malicious file.
   * @param path The path to the detected file.
   * @param hash The calculated hash of the file.
   * @param verdict The verdict from the hash database.
   */
  virtual void LogDetection(const std::filesystem::path& path,
                            const std::string& hash,
                            const std::string& verdict) = 0;
};

/**
 * @struct ScannerConfig
 * @brief Configuration structure for creating a scanner instance.
 *
 * This struct holds references to all the necessary dependencies (database,
 * logger, hasher) required by the scanner.
 */
struct SCANNER_API ScannerConfig {
  IHashDatabase& db;
  ILogger& logger;
  IFileHasher& hasher;
  std::size_t num_threads = 0;  // 0 means default to hardware_concurrency
};

/**
 * @interface IScanner
 * @brief Defines the primary contract for the file scanning engine.
 */
class SCANNER_API IScanner {
public:
  virtual ~IScanner() = default;

  /**
   * @brief Recursively scans a directory for malicious files.
   *
   * This method orchestrates the entire scanning process, utilizing multiple
   * threads to hash files and check them against the database.
   *
   * @param scan_path The root directory to begin the scan from.
   * @return A ScanResult struct containing the statistics of the completed
   * scan.
   */
  virtual ScanResult Scan(const std::filesystem::path& scan_path) = 0;
};

/**
 * @brief Factory function to create a scanner instance.
 *
 * This is the sole entry point for creating a scanner.
 *
 * @param config The configuration containing all necessary dependencies.
 * @return A unique pointer to an IScanner instance.
 */
SCANNER_API std::unique_ptr<IScanner> CreateScanner(
    const ScannerConfig& config);

}  // namespace scanner

#endif  // SCANNER_INTERFACES_H_