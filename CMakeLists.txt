cmake_minimum_required(VERSION 3.14)
project(FileScanner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(
  md5-lib
  GIT_REPOSITORY https://github.com/GregoryKogan/md5-lib.git
  GIT_TAG        v1.1.0
)
FetchContent_MakeAvailable(md5-lib)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)


message(STATUS "FileScanner project configured. Build with 'cmake --build .'")