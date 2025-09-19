add_library(scanner_lib SHARED
)


set_target_properties(scanner_lib PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    DEFINE_SYMBOL SCANNER_LIB_EXPORT
)

target_include_directories(scanner_lib PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
)
