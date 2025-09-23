#include "src/scanner_lib/file_logger.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace scanner {

FileLogger::FileLogger(const std::filesystem::path& log_path) {
  log_stream_.open(log_path, std::ios::out | std::ios::app);
  if (!log_stream_) {
    throw std::runtime_error("Failed to open log file for writing: " +
                             log_path.string());
  }
}

void FileLogger::LogDetection(const std::filesystem::path& path,
                              const std::string& hash,
                              const std::string& verdict) {
  std::stringstream json_line;
  json_line << "{\"path\": " << std::quoted(path.string(), '"', '\\')
            << ", \"hash\": " << std::quoted(hash)
            << ", \"verdict\": " << std::quoted(verdict) << "}";

  const std::lock_guard<std::mutex> lock(mutex_);
  log_stream_ << json_line.str() << std::endl;
}

}  // namespace scanner