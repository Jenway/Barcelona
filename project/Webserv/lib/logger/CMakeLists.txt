# /lib/logger/CMakeLists.txt

add_library(logger INTERFACE)

target_compile_definitions(logger INTERFACE
    PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}/"
)

target_include_directories(logger INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(logger INTERFACE
    common_lib
    utils_lib
    fmt::fmt
)
