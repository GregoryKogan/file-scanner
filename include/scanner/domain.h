#ifndef SCANNER_DOMAIN_H_
#define SCANNER_DOMAIN_H_

#include <chrono>
#include <cstdint>

#include <string>

namespace scanner {

/**
 * @struct MaliciousSignature
 * @brief Represents a single malicious signature, mapping a hash to a verdict.
 */
struct MaliciousSignature {
  std::string hash;
  std::string verdict;
};

/**
 * @struct ScanResult
 * @brief Holds the final statistics of a completed scan operation.
 */
struct ScanResult {
  std::uint64_t total_files_processed = 0;
  std::uint64_t malicious_files_detected = 0;
  std::uint64_t errors = 0;
  std::chrono::milliseconds execution_time{0};
};

}  // namespace scanner

#endif  // SCANNER_DOMAIN_H_