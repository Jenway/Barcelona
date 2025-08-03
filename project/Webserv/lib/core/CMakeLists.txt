# lib/core/CMakeLists.txt

add_library(core_lib
    src/Connection.cc
    src/Poller.cc
)

add_library(webserv::core ALIAS core_lib)

target_include_directories(core_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(core_lib
    PUBLIC
    webserv::common
    webserv::io
    webserv::tcp
    PRIVATE
    logger
)
