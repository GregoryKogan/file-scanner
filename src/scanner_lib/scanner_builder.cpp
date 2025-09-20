#include "src/scanner_lib/scanner_builder.h"

#include <stdexcept>

#include "src/scanner_lib/csv_hash_database.h"
#include "src/scanner_lib/file_logger.h"
#include "src/scanner_lib/md5_file_hasher.h"
#include "src/scanner_lib/scanner.h"

namespace scanner {

std::unique_ptr<IScannerBuilder> CreateScannerBuilder() {
  return std::make_unique<ScannerBuilder>();
}

IScannerBuilder& ScannerBuilder::WithCsvDatabase(
    const std::filesystem::path& path) {
  auto db = std::make_unique<CsvHashDatabase>();
  db->Load(path);
  db_ = std::move(db);
  return *this;
}

IScannerBuilder& ScannerBuilder::WithFileLogger(
    const std::filesystem::path& path) {
  logger_ = std::make_unique<FileLogger>(path);
  return *this;
}

IScannerBuilder& ScannerBuilder::WithMd5Hasher() {
  hasher_ = std::make_unique<Md5FileHasher>();
  return *this;
}

IScannerBuilder& ScannerBuilder::WithThreads(std::size_t num_threads) {
  num_threads_ = num_threads;
  return *this;
}

std::unique_ptr<IScanner> ScannerBuilder::Build() {
  if (!db_ || !logger_ || !hasher_) {
    throw std::runtime_error(
        "Cannot build scanner: All dependencies (database, logger, hasher) "
        "must be configured.");
  }

  return std::make_unique<Scanner>(*db_, *logger_, *hasher_, num_threads_);
}

}  // namespace scanner