# in lib/tcp/CMakeLists.txt

add_library(tcp_lib
    src/Socket.cc
    src/Acceptor.cc
    src/TcpSinker.cc
    src/TcpSource.cc
)

add_library(webserv::tcp ALIAS tcp_lib)

target_include_directories(tcp_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(tcp_lib
    PUBLIC
    webserv::common
    webserv::io
    PRIVATE
    logger
    webserv::utils
)
