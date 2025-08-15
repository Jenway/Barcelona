# lib/http_protocol/CMakeLists.txt
file(GLOB_RECURSE SOURCES "src/*.cc")
add_library(http_protocol_lib ${SOURCES})
add_library(webserv::http_protocol ALIAS http_protocol_lib)

target_include_directories(http_protocol_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(http_protocol_lib PUBLIC
    webserv::common
    webserv::io
    webserv::utils
    webserv::http_common
    webserv::http_utils
)
