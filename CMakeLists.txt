cmake_minimum_required(VERSION 3.14)
project(FileScanner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Python3 COMPONENTS Interpreter)

include(FetchContent)
FetchContent_Declare(
  md5-lib
  GIT_REPOSITORY https://github.com/GregoryKogan/md5-lib.git
  GIT_TAG        v1.1.0
)
FetchContent_MakeAvailable(md5-lib)
set_target_properties(md5_lib PROPERTIES POSITION_INDEPENDENT_CODE ON)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)

if(Python3_Interpreter_FOUND)
    add_custom_target(benchmark
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/benchmark.py $<TARGET_FILE:scanner>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running performance benchmark..."
        VERBATIM
    )
    add_dependencies(benchmark scanner)
else()
    message(WARNING "Python 3 interpreter not found. 'benchmark' target will not be available.")
endif()


message(STATUS "FileScanner project configured. Build with 'cmake --build .'")