cmake_policy(SET CMP0135 NEW)  # CMP0135: Set DOWNLOAD_EXTRACT_TIMESTAMP

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(scanner_tests
    md5_file_hasher_test.cpp
    ../src/scanner_lib/md5_file_hasher.cpp

    csv_hash_database_test.cpp
    ../src/scanner_lib/csv_hash_database.cpp

    file_logger_test.cpp
    ../src/scanner_lib/file_logger.cpp

    thread_pool_test.cpp
    ../src/scanner_lib/thread_pool.cpp
)

target_include_directories(scanner_tests PRIVATE
    "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(scanner_tests PRIVATE
    scanner_lib
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(scanner_tests)
