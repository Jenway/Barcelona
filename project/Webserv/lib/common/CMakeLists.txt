# in lib/common/CMakeLists.txt

add_library(common_lib INTERFACE)

add_library(webserv::common ALIAS common_lib)

target_include_directories(common_lib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(common_lib INTERFACE
    fmt::fmt
    magic_enum::magic_enum
)
