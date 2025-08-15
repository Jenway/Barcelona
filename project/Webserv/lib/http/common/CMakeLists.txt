# lib/http_dispatch/CMakeLists.txt
add_library(http_common_lib INTERFACE)
add_library(webserv::http_common ALIAS http_common_lib)

target_compile_definitions(http_common_lib INTERFACE MAGIC_ENUM_RANGE_MAX=1024)

target_include_directories(http_common_lib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    http_common_lib INTERFACE
    webserv::logger
)
