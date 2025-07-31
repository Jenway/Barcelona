# in lib/tcp/CMakeLists.txt

add_library(tcp_lib
    src/Socket.cc
    src/Acceptor.cc
)

target_include_directories(tcp_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(tcp_lib
    PUBLIC
    common_lib
    PRIVATE
    logger
    utils_lib
)
