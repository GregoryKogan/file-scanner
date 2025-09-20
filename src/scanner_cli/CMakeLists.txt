add_executable(scanner
    main.cpp
)

target_link_libraries(scanner PRIVATE scanner_lib)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Compiler is GCC, linking stdc++fs for std::filesystem support.")
    target_link_libraries(scanner PRIVATE stdc++fs)
endif()