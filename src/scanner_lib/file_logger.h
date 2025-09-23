#ifndef SRC_SCANNER_LIB_FILE_LOGGER_H_
#define SRC_SCANNER_LIB_FILE_LOGGER_H_

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>

#include "scanner/interfaces.h"

namespace scanner {

/**
 * @class FileLogger
 * @brief An implementation of ILogger that writes detections to a file.
 *
 * This class is thread-safe. It uses a std::mutex to synchronize access to
 * the output file stream, ensuring that log messages from concurrent threads
 * are not interleaved or corrupted. The file is opened upon construction and
 * closed upon destruction.
 */
class FileLogger final : public ILogger {
public:
  /**
   * @brief Constructs a FileLogger and opens the specified log file.
   * @param log_path The path to the log file.
   * @throws std::runtime_error if the file cannot be opened for writing.
   */
  explicit FileLogger(const std::filesystem::path& log_path);

  /**
   * @brief Logs a malicious file detection to the file in a thread-safe manner.
   * @param path The path to the detected file.
   * @param hash The calculated hash of the file.
   * @param verdict The verdict from the hash database.
   */
  void LogDetection(const std::filesystem::path& path, const std::string& hash,
                    const std::string& verdict) override;

private:
  std::ofstream log_stream_;
  std::mutex mutex_;
};

}  // namespace scanner

#endif  // SRC_SCANNER_LIB_FILE_LOGGER_H_