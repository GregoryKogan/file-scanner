#include "src/scanner_lib/scanner.h"

#include <chrono>

#include <atomic>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>

#include "src/scanner_lib/thread_pool.h"

namespace scanner {

std::unique_ptr<IScanner> CreateScanner(const ScannerConfig& config) {
  return std::make_unique<Scanner>(config);
}

Scanner::Scanner(const ScannerConfig& config)
    : db_(config.db),
      logger_(config.logger),
      hasher_(config.hasher),
      num_threads_(config.num_threads) {
}

void Scanner::ConsumerTask(const std::filesystem::path& path) {
  try {
    const std::string hash = hasher_.HashFile(path);
    const auto verdict = db_.FindHash(hash);
    if (verdict) {
      logger_.LogDetection(path, hash, *verdict);
      malicious_files_detected_++;
    }
  } catch (const std::exception& e) {
    std::cerr << "Error processing file " << path.string() << ": " << e.what()
              << std::endl;
    errors_++;
  }
  total_files_processed_++;
}

void Scanner::ProducerTask(const std::filesystem::path& scan_path,
                           ThreadPool& pool,
                           std::promise<void>& producer_promise) {
  try {
    if (!std::filesystem::exists(scan_path) ||
        !std::filesystem::is_directory(scan_path)) {
      throw std::runtime_error("Invalid scan path: " + scan_path.string());
    }

    const auto iter_options =
        std::filesystem::directory_options::skip_permission_denied;
    for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(
             scan_path, iter_options)) {
      if (dir_entry.is_regular_file()) {
        pool.Enqueue(&Scanner::ConsumerTask, this, dir_entry.path());
      }
    }
    producer_promise.set_value();  // Signal successful completion.
  } catch (const std::exception& e) {
    std::cerr << "Error during directory traversal: " << e.what() << std::endl;
    producer_promise.set_exception(std::current_exception());
  }
}

ScanResult Scanner::Scan(const std::filesystem::path& scan_path) {
  const auto start_time = std::chrono::steady_clock::now();
  total_files_processed_.store(0);
  malicious_files_detected_.store(0);
  errors_.store(0);

  {  // Inner scope to control the ThreadPool's lifetime
    ThreadPool pool(num_threads_);
    std::promise<void> producer_promise;
    auto producer_future = producer_promise.get_future();

    std::thread producer_thread(&Scanner::ProducerTask, this, scan_path,
                                std::ref(pool), std::ref(producer_promise));

    producer_thread.join();

    try {
      producer_future.get();
    } catch (const std::exception&) {
      // Error is already logged.
    }
  }

  const auto end_time = std::chrono::steady_clock::now();
  ScanResult result;
  result.total_files_processed = total_files_processed_.load();
  result.malicious_files_detected = malicious_files_detected_.load();
  result.errors = errors_.load();
  result.execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(
      end_time - start_time);
  return result;
}

}  // namespace scanner