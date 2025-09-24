#include "src/scanner_lib/scanner.h"

#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "scanner/interfaces.h"
#include "src/scanner_lib/thread_pool.h"

namespace scanner {
namespace {

// --- Mocks ---

class MockFileHasher : public IFileHasher {
public:
  MOCK_METHOD(std::string, HashFile, (const std::filesystem::path& file_path),
              (override));
};

class MockHashDatabase : public IHashDatabase {
public:
  MOCK_METHOD(std::size_t, Load, (const std::filesystem::path& source_path),
              (override));
  MOCK_METHOD(std::optional<std::string>, FindHash, (const std::string& hash),
              (const, override));
};

class MockLogger : public ILogger {
public:
  MOCK_METHOD(void, LogDetection,
              (const std::filesystem::path& path, const std::string& hash,
               const std::string& verdict),
              (override));
};

// --- Test Fixture ---

class ScannerTest : public ::testing::Test {
protected:
  void SetUp() override {
    const std::string test_name =
        ::testing::UnitTest::GetInstance()->current_test_info()->name();
    temp_dir_ = std::filesystem::temp_directory_path() /
                ("scanner_scan_tests_" + test_name);
    std::filesystem::create_directories(temp_dir_);
  }

  void TearDown() override {
    std::error_code ec;
    std::filesystem::remove_all(temp_dir_, ec);
    (void)ec;
  }

  void CreateDummyFile(const std::filesystem::path& path) {
    std::ofstream f(temp_dir_ / path);
    f << "dummy content";
  }

  testing::StrictMock<MockFileHasher> mock_hasher_;
  testing::StrictMock<MockHashDatabase> mock_db_;
  testing::StrictMock<MockLogger> mock_logger_;

  std::filesystem::path temp_dir_;
};

// --- Test Cases ---

TEST_F(ScannerTest, FindsAndLogsMaliciousFiles) {
  CreateDummyFile("good_file.txt");
  CreateDummyFile("bad_file.exe");

  EXPECT_CALL(mock_hasher_, HashFile(temp_dir_ / "good_file.txt"))
      .WillOnce(testing::Return("good_hash"));
  EXPECT_CALL(mock_hasher_, HashFile(temp_dir_ / "bad_file.exe"))
      .WillOnce(testing::Return("bad_hash"));

  EXPECT_CALL(mock_db_, FindHash("good_hash"))
      .WillOnce(testing::Return(std::nullopt));
  EXPECT_CALL(mock_db_, FindHash("bad_hash"))
      .WillOnce(testing::Return("EvilWare"));

  EXPECT_CALL(mock_logger_,
              LogDetection(temp_dir_ / "bad_file.exe", "bad_hash", "EvilWare"))
      .Times(1);

  auto scanner =
      std::make_unique<Scanner>(mock_db_, mock_logger_, mock_hasher_, 2);
  const ScanResult result = scanner->Scan(temp_dir_);

  EXPECT_EQ(result.total_files_processed, 2);
  EXPECT_EQ(result.malicious_files_detected, 1);
  EXPECT_EQ(result.errors, 0);
}

TEST_F(ScannerTest, HandlesHashingErrorsGracefully) {
  CreateDummyFile("good_file.txt");
  CreateDummyFile("permission_denied.sys");

  EXPECT_CALL(mock_hasher_, HashFile(temp_dir_ / "good_file.txt"))
      .WillOnce(testing::Return("good_hash"));
  EXPECT_CALL(mock_hasher_, HashFile(temp_dir_ / "permission_denied.sys"))
      .WillOnce(testing::Throw(std::runtime_error("Permission denied")));

  EXPECT_CALL(mock_db_, FindHash("good_hash"))
      .WillOnce(testing::Return(std::nullopt));

  EXPECT_CALL(mock_logger_, LogDetection(testing::_, testing::_, testing::_))
      .Times(0);

  auto scanner =
      std::make_unique<Scanner>(mock_db_, mock_logger_, mock_hasher_, 2);
  const ScanResult result = scanner->Scan(temp_dir_);

  EXPECT_EQ(result.total_files_processed, 2);
  EXPECT_EQ(result.malicious_files_detected, 0);
  EXPECT_EQ(result.errors, 1);
}

TEST_F(ScannerTest, HandlesEmptyDirectory) {
  EXPECT_CALL(mock_hasher_, HashFile(testing::_)).Times(0);
  EXPECT_CALL(mock_db_, FindHash(testing::_)).Times(0);
  EXPECT_CALL(mock_logger_, LogDetection(testing::_, testing::_, testing::_))
      .Times(0);

  auto scanner =
      std::make_unique<Scanner>(mock_db_, mock_logger_, mock_hasher_, 2);
  const ScanResult result = scanner->Scan(temp_dir_);

  EXPECT_EQ(result.total_files_processed, 0);
  EXPECT_EQ(result.malicious_files_detected, 0);
  EXPECT_EQ(result.errors, 0);
}

TEST_F(ScannerTest, HandlesDeeplyNestedDirectories) {
  const auto dir_a = temp_dir_ / "a";
  const auto dir_b = dir_a / "b";
  const auto dir_c = dir_b / "c";
  std::filesystem::create_directories(dir_c);

  CreateDummyFile("file1.txt");
  CreateDummyFile(dir_a / "file2.txt");
  CreateDummyFile(dir_b / "file3.txt");
  CreateDummyFile(dir_c / "file4.txt");

  EXPECT_CALL(mock_hasher_, HashFile(testing::_))
      .Times(4)
      .WillRepeatedly(testing::Return("some_hash"));

  EXPECT_CALL(mock_db_, FindHash("some_hash"))
      .Times(4)
      .WillRepeatedly(testing::Return(std::nullopt));

  EXPECT_CALL(mock_logger_, LogDetection(testing::_, testing::_, testing::_))
      .Times(0);

  auto scanner =
      std::make_unique<Scanner>(mock_db_, mock_logger_, mock_hasher_, 4);
  const ScanResult result = scanner->Scan(temp_dir_);

  EXPECT_EQ(result.total_files_processed, 4);
  EXPECT_EQ(result.malicious_files_detected, 0);
  EXPECT_EQ(result.errors, 0);
}

TEST_F(ScannerTest, HandlesInvalidScanPath) {
  const auto invalid_path = temp_dir_ / "not_a_directory.txt";
  CreateDummyFile("not_a_directory.txt");
  ASSERT_TRUE(std::filesystem::is_regular_file(invalid_path));

  EXPECT_CALL(mock_hasher_, HashFile(testing::_)).Times(0);
  EXPECT_CALL(mock_db_, FindHash(testing::_)).Times(0);
  EXPECT_CALL(mock_logger_, LogDetection(testing::_, testing::_, testing::_))
      .Times(0);

  Scanner scanner(mock_db_, mock_logger_, mock_hasher_, 2);
  const ScanResult result = scanner.Scan(invalid_path);

  EXPECT_GE(result.errors, 1);
}

}  // namespace
}  // namespace scanner