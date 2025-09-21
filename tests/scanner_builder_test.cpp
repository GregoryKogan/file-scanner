#include <filesystem>
#include <fstream>

#include "gtest/gtest.h"
#include "scanner/interfaces.h"

namespace scanner {
namespace {

class ScannerBuilderTest : public ::testing::Test {
protected:
  void SetUp() override {
    temp_dir_ = std::filesystem::temp_directory_path() / "builder_tests";
    std::filesystem::create_directory(temp_dir_);
    db_path_ = temp_dir_ / "db.csv";
    log_path_ = temp_dir_ / "log.txt";
    std::ofstream(db_path_) << "hash;verdict\n";
  }

  void TearDown() override {
    std::error_code ec;
    std::filesystem::remove_all(temp_dir_, ec);
    (void)ec;
  }

  std::filesystem::path temp_dir_;
  std::filesystem::path db_path_;
  std::filesystem::path log_path_;
};

TEST_F(ScannerBuilderTest, BuildSucceedsWithAllDependencies) {
  auto builder = CreateScannerBuilder();
  EXPECT_NO_THROW({
    builder->WithCsvDatabase(db_path_)
        .WithFileLogger(log_path_)
        .WithMd5Hasher()
        .WithThreads(4);
    auto scanner = builder->Build();
    EXPECT_NE(scanner, nullptr);
  });
}

TEST_F(ScannerBuilderTest, BuildThrowsWithoutDatabase) {
  auto builder = CreateScannerBuilder();
  builder->WithFileLogger(log_path_).WithMd5Hasher();
  EXPECT_THROW(builder->Build(), std::runtime_error);
}

TEST_F(ScannerBuilderTest, BuildThrowsWithoutLogger) {
  auto builder = CreateScannerBuilder();
  builder->WithCsvDatabase(db_path_).WithMd5Hasher();
  EXPECT_THROW(builder->Build(), std::runtime_error);
}

TEST_F(ScannerBuilderTest, BuildThrowsWithoutHasher) {
  auto builder = CreateScannerBuilder();
  builder->WithCsvDatabase(db_path_).WithFileLogger(log_path_);
  EXPECT_THROW(builder->Build(), std::runtime_error);
}

}  // namespace
}  // namespace scanner