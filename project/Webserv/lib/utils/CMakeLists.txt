add_library(utils_lib INTERFACE)

target_link_libraries(utils_lib INTERFACE common_lib)

target_include_directories(utils_lib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
