# lib/http/CMakeLists.txt

add_library(http_lib
    src/HttpProtocolHandler.cc
    src/RequestParser.cc
    src/RequestRouter.cc
    src/ResponseFactory.cc
    src/ResponseWriter.cc
)

add_library(webserv::http ALIAS http_lib)

target_compile_definitions(http_lib PUBLIC MAGIC_ENUM_RANGE_MAX=1024)

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
