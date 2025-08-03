add_library(utils_lib INTERFACE)

add_library(webserv::utils ALIAS utils_lib)

target_link_libraries(utils_lib INTERFACE webserv::common)

target_include_directories(utils_lib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
