#ifndef TESTS_PROCESS_HELPER_H_
#define TESTS_PROCESS_HELPER_H_

#include <cstdio>

#include <array>
#include <memory>
#include <stdexcept>
#include <string>

namespace tests {

/**
 * @brief Executes a command as a subprocess and captures its combined output.
 *
 * A simple, cross-platform wrapper around popen/_popen to run a command
 * and read its stdout and stderr streams.
 *
 * @param command The full command line to execute.
 * @return A string containing the captured stdout and stderr of the command.
 * @throws std::runtime_error if the process fails to start.
 */
inline std::string Execute(const std::string& command) {
  std::array<char, 128> buffer;
  std::string result;

  // Append stderr redirection to the command to capture it along with stdout.
  const std::string cmd_with_stderr = command + " 2>&1";

#ifdef _WIN32
  std::unique_ptr<FILE, decltype(&_pclose)> pipe(
      _popen(cmd_with_stderr.c_str(), "r"), _pclose);
#else
  std::unique_ptr<FILE, decltype(&pclose)> pipe(
      popen(cmd_with_stderr.c_str(), "r"), pclose);
#endif

  if (!pipe) {
    throw std::runtime_error("popen() failed for command: " + command);
  }

  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }

  return result;
}

}  // namespace tests

#endif  // TESTS_PROCESS_HELPER_H_