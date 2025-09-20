#ifndef SCANNER_DOMAIN_H_
#define SCANNER_DOMAIN_H_

#include <chrono>
#include <cstdint>

#include <iostream>
#include <string>

#include "scanner/visibility.h"

namespace scanner {

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

/**
 * @brief Overload for streaming a ScanResult to an output stream.
 * @param os The output stream (e.g., std::cout).
 * @param result The ScanResult to print.
 * @return A reference to the output stream.
 */
SCANNER_API std::ostream& operator<<(std::ostream& os,
                                     const ScanResult& result);

}  // namespace scanner

#endif  // SCANNER_DOMAIN_H_