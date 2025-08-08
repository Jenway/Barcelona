# lib/http_dispatch/CMakeLists.txt
add_library(http_utils_lib
    src/ResponseFactory.cc
)
add_library(webserv::http_utils ALIAS http_utils_lib)

target_include_directories(http_utils_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(http_utils_lib PUBLIC
    webserv::utils
    webserv::http_common
)
