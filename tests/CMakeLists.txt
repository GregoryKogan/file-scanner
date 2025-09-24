cmake_policy(SET CMP0135 NEW)  # CMP0135: Set DOWNLOAD_EXTRACT_TIMESTAMP

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


# --- Unit Tests ---

add_executable(scanner_tests
    md5_file_hasher_test.cpp
    ../src/scanner_lib/md5_file_hasher.cpp

    csv_hash_database_test.cpp
    ../src/scanner_lib/csv_hash_database.cpp

    file_logger_test.cpp
    ../src/scanner_lib/file_logger.cpp

    thread_pool_test.cpp
    ../src/scanner_lib/thread_pool.cpp

    scanner_test.cpp
    ../src/scanner_lib/scanner.cpp

    scanner_builder_test.cpp
    ../src/scanner_lib/scanner_builder.cpp
)

target_include_directories(scanner_tests PRIVATE
    "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(scanner_tests PRIVATE
    scanner_lib
    GTest::gtest_main
    GTest::gmock
)

# On POSIX systems it is required to link against the threads library.
find_package(Threads REQUIRED)
target_link_libraries(scanner_tests PRIVATE Threads::Threads)

include(GoogleTest)
gtest_discover_tests(scanner_tests)


# --- Integration Tests ---

add_executable(integration_tests
    integration_test.cpp
)

target_link_libraries(integration_tests PRIVATE
    GTest::gtest_main
    GTest::gmock
)

# Pass the location of the 'scanner' executable to the integration test
# source code as a preprocessor definition.
target_compile_definitions(integration_tests PRIVATE
    SCANNER_EXECUTABLE_PATH="$<TARGET_FILE:scanner>"
)

target_include_directories(integration_tests PRIVATE
    "${PROJECT_SOURCE_DIR}"
)

add_test(NAME integration_tests COMMAND $<TARGET_FILE:integration_tests>)

add_dependencies(integration_tests scanner)