# lib/http_handlers/CMakeLists.txt
file(GLOB_RECURSE SOURCES "src/*.cc")
add_library(http_handlers_lib ${SOURCES})
add_library(webserv::http_handlers ALIAS http_handlers_lib)
target_include_directories(http_handlers_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(http_handlers_lib PUBLIC
    webserv::http_common
    webserv::http_utils
)
