#include <cstdlib>

#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "scanner/domain.h"
#include "scanner/interfaces.h"

namespace {
struct Args;
void PrintUsage();
Args ParseArgs(int argc, char* argv[]);
}  // namespace

int main(int argc, char* argv[]) {
  std::cout << "Starting file scan...\n";

  const Args args = ParseArgs(argc, argv);

  try {
    auto builder = scanner::CreateScannerBuilder();

    std::cout << "Configuring scanner...\n";
    builder->WithCsvDatabase(args.base_path)
        .WithFileLogger(args.log_path)
        .WithMd5Hasher();

    auto scanner = builder->Build();

    std::cout << "Scanning directory: " << args.scan_path << "\n";
    const scanner::ScanResult result = scanner->Scan(args.scan_path);

    std::cout << "\n" << result << std::endl;
  } catch (const std::exception& e) {
    std::cerr << "A critical error occurred: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

namespace {

struct Args {
  std::filesystem::path scan_path;
  std::filesystem::path base_path;
  std::filesystem::path log_path;
};

void PrintUsage() {
  std::cout
      << "Usage: scanner.exe --path <scan_directory> --base <database.csv> "
         "--log <report.log>\n";
}

Args ParseArgs(int argc, char* argv[]) {
  if (argc != 7) {
    PrintUsage();
    exit(EXIT_FAILURE);
  }

  std::unordered_map<std::string, std::string> args_map;
  for (int i = 1; i < argc; i += 2) {
    args_map[argv[i]] = argv[i + 1];
  }

  Args args;
  try {
    args.scan_path = args_map.at("--path");
    args.base_path = args_map.at("--base");
    args.log_path = args_map.at("--log");
  } catch (const std::out_of_range&) {
    PrintUsage();
    exit(EXIT_FAILURE);
  }

  if (!std::filesystem::exists(args.scan_path) ||
      !std::filesystem::is_directory(args.scan_path)) {
    std::cerr << "Error: Scan path does not exist or is not a directory: "
              << args.scan_path << std::endl;
    exit(EXIT_FAILURE);
  }

  if (!std::filesystem::exists(args.base_path) ||
      !std::filesystem::is_regular_file(args.base_path)) {
    std::cerr << "Error: Hash database file does not exist or is not a file: "
              << args.base_path << std::endl;
    exit(EXIT_FAILURE);
  }

  const auto log_parent_dir = args.log_path.parent_path();
  if (!log_parent_dir.empty() && !std::filesystem::exists(log_parent_dir)) {
    std::cerr << "Error: Log file's parent directory does not exist: "
              << log_parent_dir << std::endl;
    exit(EXIT_FAILURE);
  }

  return args;
}

}  // namespace