# lib/http/CMakeLists.txt

add_library(http_lib
    src/ResponseWriter.cc
    src/RequestParser.cc
    src/HttpProtocolHandler.cc
)

add_library(webserv::http ALIAS http_lib)

target_include_directories(http_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(http_lib
    PUBLIC
    webserv::common
    webserv::io
    PRIVATE
    logger
)
