# lib/config/CMakeLists.txt

add_library(config_lib
    src/ConfigLoader.cc
)

add_library(webserv::config ALIAS config_lib)

target_include_directories(config_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(config_lib PUBLIC
    webserv::common
    nlohmann_json::nlohmann_json
)
