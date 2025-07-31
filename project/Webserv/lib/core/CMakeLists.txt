# lib/core/CMakeLists.txt

add_library(core_lib
    src/Connection.cc
)

target_include_directories(core_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(core_lib
    PUBLIC
    common_lib
    io_lib
    tcp_lib
    PRIVATE
    logger
)
