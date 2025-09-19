#include "src/scanner_lib/csv_hash_database.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "gtest/gtest.h"

namespace scanner {
namespace {

class CsvHashDatabaseTest : public ::testing::Test {
protected:
  void SetUp() override {
    const std::string test_name =
        ::testing::UnitTest::GetInstance()->current_test_info()->name();
    temp_dir_ = std::filesystem::temp_directory_path() /
                ("scanner_db_tests_" + test_name);
    std::filesystem::create_directory(temp_dir_);
  }

  void TearDown() override {
    std::error_code ec;
    std::filesystem::remove_all(temp_dir_, ec);
  }

  std::filesystem::path CreateDbFile(const std::string& filename,
                                     const std::string& content) const {
    const std::filesystem::path file_path = temp_dir_ / filename;
    std::ofstream db_file(file_path);
    db_file << content;
    db_file.close();
    return file_path;
  }

  std::filesystem::path temp_dir_;
};

TEST_F(CsvHashDatabaseTest, LoadsValidFile) {
  const std::string content =
      "a;Verdict1\n"
      "b;Verdict2\n"
      "c;Verdict3";
  const auto db_path = CreateDbFile("valid.csv", content);

  CsvHashDatabase db;
  EXPECT_EQ(db.Load(db_path), 3);

  EXPECT_EQ(db.FindHash("a").value_or(""), "Verdict1");
  EXPECT_EQ(db.FindHash("b").value_or(""), "Verdict2");
  EXPECT_EQ(db.FindHash("c").value_or(""), "Verdict3");
  EXPECT_FALSE(db.FindHash("d").has_value());
}

TEST_F(CsvHashDatabaseTest, HandlesEmptyFile) {
  const auto db_path = CreateDbFile("empty.csv", "");
  CsvHashDatabase db;
  EXPECT_EQ(db.Load(db_path), 0);
  EXPECT_FALSE(db.FindHash("a").has_value());
}

TEST_F(CsvHashDatabaseTest, ClearsPreviousDataOnLoad) {
  const auto db_path1 = CreateDbFile("db1.csv", "a;VerdictA");
  CsvHashDatabase db;
  db.Load(db_path1);
  ASSERT_TRUE(db.FindHash("a").has_value());

  const auto db_path2 = CreateDbFile("db2.csv", "b;VerdictB");
  EXPECT_EQ(db.Load(db_path2), 1);
  EXPECT_FALSE(db.FindHash("a").has_value());
  EXPECT_TRUE(db.FindHash("b").has_value());
}

TEST_F(CsvHashDatabaseTest, SkipsMalformedLines) {
  const std::string content =
      "a;Verdict1\n"        // Valid
      "b;\n"                // Malformed (empty verdict)
      ";Verdict2\n"         // Malformed (empty hash)
      "c;Verdict3;Extra\n"  // Malformed (too many parts)
      "d\n"                 // Malformed (too few parts)
      "e;Verdict5";         // Valid
  const auto db_path = CreateDbFile("malformed.csv", content);

  CsvHashDatabase db;
  EXPECT_EQ(db.Load(db_path), 2);

  EXPECT_TRUE(db.FindHash("a").has_value());
  EXPECT_FALSE(db.FindHash("b").has_value());
  EXPECT_FALSE(db.FindHash("c").has_value());
  EXPECT_FALSE(db.FindHash("d").has_value());
  EXPECT_TRUE(db.FindHash("e").has_value());
}

TEST_F(CsvHashDatabaseTest, ThrowsOnNonExistentFile) {
  CsvHashDatabase db;
  const std::filesystem::path non_existent_path =
      temp_dir_ / "no_such_file.csv";
  EXPECT_THROW(static_cast<void>(db.Load(non_existent_path)),
               std::runtime_error);
}

}  // namespace
}  // namespace scanner