# lib/io/CMakeLists.txt
add_library(io_lib INTERFACE)

add_library(webserv::io ALIAS io_lib)

target_include_directories(io_lib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(io_lib INTERFACE
    webserv::common
)
