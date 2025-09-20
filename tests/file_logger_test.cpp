#include "src/scanner_lib/file_logger.h"

#include <atomic>
#include <filesystem>
#include <fstream>
#include <future>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

namespace scanner {
namespace {

class FileLoggerTest : public ::testing::Test {
protected:
  void SetUp() override {
    const std::string test_name =
        ::testing::UnitTest::GetInstance()->current_test_info()->name();
    temp_dir_ = std::filesystem::temp_directory_path() /
                ("scanner_log_tests_" + test_name);
    std::filesystem::create_directory(temp_dir_);
  }

  void TearDown() override {
    std::error_code ec;
    std::filesystem::remove_all(temp_dir_, ec);
    (void)ec;
  }

  std::filesystem::path temp_dir_;
};

TEST_F(FileLoggerTest, LogsSingleEntryCorrectly) {
  const auto log_path = temp_dir_ / "single.log";
  {
    FileLogger logger(log_path);
    logger.LogDetection(R"(c:\temp\file.txt)", "hash1", "Verdict1");
  }  // Logger goes out of scope, closes file.

  std::ifstream log_file(log_path);
  std::string line;
  ASSERT_TRUE(std::getline(log_file, line));

  const std::string expected =
      R"({"path": "c:\\temp\\file.txt", "hash": "hash1", "verdict": "Verdict1"})";
  EXPECT_EQ(line, expected);

  EXPECT_FALSE(std::getline(log_file, line));  // No more lines
}

TEST_F(FileLoggerTest, ThrowsOnNonExistentDirectory) {
  const auto non_existent_dir = temp_dir_ / "this_dir_does_not_exist";
  const auto log_path = non_existent_dir / "test.log";

  ASSERT_FALSE(std::filesystem::exists(non_existent_dir));

  EXPECT_THROW(FileLogger logger(log_path), std::runtime_error);
}

TEST_F(FileLoggerTest, HandlesConcurrentWritesWithoutCorruption) {
  const auto log_path = temp_dir_ / "concurrent.log";
  constexpr int kNumThreads = 16;
  constexpr int kLogsPerThread = 100;

  {
    FileLogger logger(log_path);
    std::vector<std::thread> threads;
    threads.reserve(kNumThreads);

    for (int i = 0; i < kNumThreads; ++i) {
      threads.emplace_back([&logger, i, kLogsPerThread]() {
        for (int j = 0; j < kLogsPerThread; ++j) {
          const std::string path =
              "file_" + std::to_string(i) + "_" + std::to_string(j);
          const std::string hash = "hash_" + std::to_string(j);
          const std::string verdict = "Verdict" + std::to_string(i);
          logger.LogDetection(path, hash, verdict);
        }
      });
    }

    for (auto& t : threads) {
      t.join();
    }
  }  // Logger is destroyed, file is closed.

  std::ifstream log_file(log_path);
  ASSERT_TRUE(log_file.is_open());
  std::string line;
  int line_count = 0;
  while (std::getline(log_file, line)) {
    line_count++;
    EXPECT_EQ(line.front(), '{');
    EXPECT_EQ(line.back(), '}');
  }

  EXPECT_EQ(line_count, kNumThreads * kLogsPerThread);
}

}  // namespace
}  // namespace scanner