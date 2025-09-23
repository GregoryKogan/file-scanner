#include "src/scanner_lib/csv_hash_database.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace scanner {
namespace {

std::vector<std::string> Split(const std::string& s, char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream token_stream(s);
  while (std::getline(token_stream, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

}  // namespace

std::size_t CsvHashDatabase::Load(const std::filesystem::path& source_path) {
  std::ifstream db_file(source_path);
  if (!db_file) {
    throw std::runtime_error("Failed to open hash database file: " +
                             source_path.string());
  }

  signatures_.clear();
  std::string line;
  std::size_t line_number = 0;
  while (std::getline(db_file, line)) {
    line_number++;
    if (line.empty()) {
      continue;
    }

    const auto parts = Split(line, ';');
    if (parts.size() != 2 || parts[0].empty() || parts[1].empty()) {
      std::cerr << "Warning: Malformed line " << line_number
                << " in database file, skipping: " << source_path.string()
                << std::endl;
      continue;
    }

    const std::string& hash = parts[0];
    const std::string& verdict = parts[1];
    signatures_[hash] = verdict;
  }

  return signatures_.size();
}

std::optional<std::string> CsvHashDatabase::FindHash(
    const std::string& hash) const {
  const auto it = signatures_.find(hash);
  if (it != signatures_.end()) {
    return it->second;
  }
  return std::nullopt;
}

}  // namespace scanner