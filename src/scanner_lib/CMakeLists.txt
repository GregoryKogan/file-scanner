add_library(scanner_lib SHARED
    md5_file_hasher.cpp
    csv_hash_database.cpp
)

set_target_properties(scanner_lib PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    DEFINE_SYMBOL SCANNER_LIB_EXPORT
)

target_include_directories(scanner_lib
    PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
    PRIVATE
        "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(scanner_lib PUBLIC md5_lib)
