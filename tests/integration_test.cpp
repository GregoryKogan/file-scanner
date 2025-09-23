#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "tests/process_helper.h"

#define STRINGIFY_IMPL(s) #s
#define STRINGIFY(s) STRINGIFY_IMPL(s)

namespace {

class ScannerIntegrationTest : public ::testing::Test {
protected:
  void SetUp() override {
    // 1. Create a unique root directory for the test.
    const std::string test_name =
        ::testing::UnitTest::GetInstance()->current_test_info()->name();
    root_dir_ = std::filesystem::temp_directory_path() /
                ("scanner_integ_tests_" + test_name);
    std::filesystem::create_directories(root_dir_);

    scan_dir_ = root_dir_ / "scan_me";
    std::filesystem::create_directory(scan_dir_);

    // 2. Define test data.
    bad_hash1_ = "179052c9c6165bf25917781fc5816993";  // MD5 of "EVIL"
    bad_hash2_ = "b867e23836356d568aadfe4a2fe9b0e1";  // MD5 of "MALWARE"

    // 3. Create the file structure to be scanned.
    const auto nested_dir = scan_dir_ / "nested";
    std::filesystem::create_directory(nested_dir);

    CreateFile("good_file1.txt", "This is a safe file.");
    CreateFile("bad_file1.exe", "EVIL");
    CreateFile(nested_dir / "good_file2.log", "Another safe file.");
    CreateFile(nested_dir / "bad_file2.dll", "MALWARE");
    CreateFile("empty_file.txt", "");

    // 4. Create the malicious hash database file.
    base_path_ = root_dir_ / "base.csv";
    std::ofstream base_file(base_path_);
    base_file << bad_hash1_ << ";Exploit\n";
    base_file << bad_hash2_ << ";Dropper\n";
    base_file.close();

    // 5. Define the path for the output log.
    log_path_ = root_dir_ / "report.log";
  }

  void TearDown() override {
    std::error_code ec;
    std::filesystem::remove_all(root_dir_, ec);
    (void)ec;
  }

  void CreateFile(const std::filesystem::path& relative_path,
                  const std::string& content) {
    std::ofstream f(scan_dir_ / relative_path);
    f << content;
  }

  std::filesystem::path root_dir_;
  std::filesystem::path scan_dir_;
  std::filesystem::path base_path_;
  std::filesystem::path log_path_;
  std::string bad_hash1_;
  std::string bad_hash2_;
};

TEST_F(ScannerIntegrationTest, FullScanDetectsThreatsAndReportsCorrectly) {
  const std::string scanner_path = STRINGIFY(SCANNER_EXECUTABLE_PATH);
  std::string command = scanner_path;
  command += " --path " + scan_dir_.string();
  command += " --base " + base_path_.string();
  command += " --log " + log_path_.string();

  const std::string console_output = tests::Execute(command);

  std::cout << "--- Scanner Console Output ---\n"
            << console_output << "\n--------------------------\n";

  ASSERT_TRUE(std::filesystem::exists(log_path_))
      << "Log file was not created.";
  std::ifstream log_file(log_path_);
  std::string log_line;
  std::vector<std::string> log_entries;
  while (std::getline(log_file, log_line)) {
    log_entries.push_back(log_line);
  }

  ASSERT_EQ(log_entries.size(), 2)
      << "Incorrect number of detections in log file.";

  EXPECT_THAT(
      log_entries,
      testing::UnorderedElementsAre(
          testing::AllOf(testing::HasSubstr("\"hash\": \"" + bad_hash1_ + "\""),
                         testing::HasSubstr(R"("verdict": "Exploit")"),
                         testing::HasSubstr("bad_file1.exe")),
          testing::AllOf(testing::HasSubstr("\"hash\": \"" + bad_hash2_ + "\""),
                         testing::HasSubstr(R"("verdict": "Dropper")"),
                         testing::HasSubstr("bad_file2.dll"))));

  EXPECT_THAT(console_output, testing::HasSubstr("Processed files: 5"));
  EXPECT_THAT(console_output, testing::HasSubstr("Malicious detections: 2"));
  EXPECT_THAT(console_output, testing::HasSubstr("Errors: 0"));
  EXPECT_THAT(console_output, testing::HasSubstr("Execution time:"));
}

}  // namespace