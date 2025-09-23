#include "scanner/domain.h"

#include <ostream>

namespace scanner {

std::ostream& operator<<(std::ostream& os, const ScanResult& result) {
  os << "--- Scan Report ---\n"
     << "Processed files: " << result.total_files_processed << "\n"
     << "Malicious detections: " << result.malicious_files_detected << "\n"
     << "Errors: " << result.errors << "\n"
     << "Execution time: " << result.execution_time.count() << " ms\n"
     << "-------------------";
  return os;
}

}  // namespace scanner