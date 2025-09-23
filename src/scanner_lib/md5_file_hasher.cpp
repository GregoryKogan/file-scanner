#include "src/scanner_lib/md5_file_hasher.h"

#include <fstream>
#include <stdexcept>

#include <md5.h>

namespace scanner {

std::string Md5FileHasher::HashFile(const std::filesystem::path& file_path) {
  std::ifstream file_stream(file_path, std::ios::binary);
  if (!file_stream) {
    throw std::runtime_error("Failed to open file: " + file_path.string());
  }

  // Set the stream to throw an exception on read errors.
  file_stream.exceptions(std::ifstream::badbit);

  return md5_lib::CalculateMD5(file_stream);
}

}  // namespace scanner