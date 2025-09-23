#include "src/scanner_lib/md5_file_hasher.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include "gtest/gtest.h"

namespace scanner {
namespace {

class Md5FileHasherTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    temp_dir_ = std::filesystem::temp_directory_path() / "scanner_tests";
    std::filesystem::create_directory(temp_dir_);

    known_content_path_ = temp_dir_ / "known_content.txt";
    std::ofstream known_file(known_content_path_);
    known_file << "hello world";
    known_file.close();

    empty_file_path_ = temp_dir_ / "empty.txt";
    std::ofstream(empty_file_path_).close();
  }

  static void TearDownTestSuite() {
    std::error_code ec;
    std::filesystem::remove_all(temp_dir_, ec);
  }

  static std::filesystem::path temp_dir_;
  static std::filesystem::path known_content_path_;
  static std::filesystem::path empty_file_path_;
};

std::filesystem::path Md5FileHasherTest::temp_dir_;
std::filesystem::path Md5FileHasherTest::known_content_path_;
std::filesystem::path Md5FileHasherTest::empty_file_path_;

TEST_F(Md5FileHasherTest, HashesKnownFileCorrectly) {
  Md5FileHasher hasher;
  const std::string expected_hash = "5eb63bbbe01eeed093cb22bb8f5acdc3";
  EXPECT_EQ(hasher.HashFile(known_content_path_), expected_hash);
}

TEST_F(Md5FileHasherTest, HashesEmptyFileCorrectly) {
  Md5FileHasher hasher;
  const std::string expected_hash = "d41d8cd98f00b204e9800998ecf8427e";
  EXPECT_EQ(hasher.HashFile(empty_file_path_), expected_hash);
}

TEST_F(Md5FileHasherTest, ThrowsOnNonExistentFile) {
  Md5FileHasher hasher;
  const std::filesystem::path non_existent_path =
      temp_dir_ / "non_existent.txt";
  EXPECT_THROW(static_cast<void>(hasher.HashFile(non_existent_path)),
               std::runtime_error);
}

}  // namespace
}  // namespace scanner