#ifndef SRC_SCANNER_LIB_SCANNER_BUILDER_H_
#define SRC_SCANNER_LIB_SCANNER_BUILDER_H_

#include <memory>

#include "scanner/interfaces.h"

namespace scanner {

class ScannerBuilder final : public IScannerBuilder {
public:
  IScannerBuilder& WithCsvDatabase(const std::filesystem::path& path) override;
  IScannerBuilder& WithFileLogger(const std::filesystem::path& path) override;
  IScannerBuilder& WithMd5Hasher() override;
  IScannerBuilder& WithThreads(std::size_t num_threads) override;
  std::unique_ptr<IScanner> Build() override;

private:
  std::unique_ptr<IHashDatabase> db_;
  std::unique_ptr<ILogger> logger_;
  std::unique_ptr<IFileHasher> hasher_;
  std::size_t num_threads_ = 0;
};

}  // namespace scanner
#endif  // SRC_SCANNER_LIB_SCANNER_BUILDER_H_