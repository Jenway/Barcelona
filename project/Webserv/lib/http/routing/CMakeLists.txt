# lib/http_dispatch/CMakeLists.txt
file(GLOB_RECURSE SOURCES "src/*.cc")
add_library(http_dispatch_lib ${SOURCES})
add_library(webserv::http_dispatch ALIAS http_dispatch_lib)
target_include_directories(http_dispatch_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(http_dispatch_lib PUBLIC
    webserv::http_common
    webserv::http_utils
)
