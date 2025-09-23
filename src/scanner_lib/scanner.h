#ifndef SRC_SCANNER_LIB_SCANNER_H_
#define SRC_SCANNER_LIB_SCANNER_H_

#include <cstdint>

#include <atomic>
#include <filesystem>
#include <future>

#include "scanner/interfaces.h"
#include "src/scanner_lib/thread_pool.h"

namespace scanner {

/**
 * @class Scanner
 * @brief The concrete, internal implementation of the IScanner interface.
 *
 * This class is not exported and is only accessible via the CreateScanner
 * factory function. It orchestrates the multithreaded scanning process.
 */
class Scanner final : public IScanner {
public:
  /**
   * @brief Constructs a Scanner instance.
   * @param config The configuration containing dependencies.
   */
  explicit Scanner(const ScannerConfig& config);

  /**
   * @brief Scans the specified directory.
   * @param scan_path The root directory for the scan.
   * @return The results of the scan.
   */
  ScanResult Scan(const std::filesystem::path& scan_path) override;

private:
  /**
   * @brief The task executed by the producer thread.
   *
   * Traverses the filesystem recursively from the given root path, enqueues
   * a consumer task for each regular file found, and signals completion or
   * error via a promise.
   *
   * @param scan_path The root directory to traverse.
   * @param pool The thread pool to enqueue tasks into.
   * @param producer_promise A promise to signal the outcome of the traversal.
   */
  void ProducerTask(const std::filesystem::path& scan_path, ThreadPool& pool,
                    std::promise<void>& producer_promise);

  /**
   * @brief The task executed by consumer threads in the pool.
   *
   * Processes a single file: hashes it, checks the hash against the database,
   * and logs a detection if found. It also updates the atomic counters for
   * scan statistics.
   *
   * @param path The path of the file to process.
   */
  void ConsumerTask(const std::filesystem::path& path);

  IHashDatabase& db_;
  ILogger& logger_;
  IFileHasher& hasher_;
  std::size_t num_threads_;

  std::atomic<std::uint64_t> total_files_processed_{0};
  std::atomic<std::uint64_t> malicious_files_detected_{0};
  std::atomic<std::uint64_t> errors_{0};
};

}  // namespace scanner

#endif  // SRC_SCANNER_LIB_SCANNER_H_