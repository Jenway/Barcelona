# /CMakeLists.txt

cmake_minimum_required(VERSION 3.20)
project(WebServer CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

enable_testing()

find_package(GTest REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(magic_enum REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_subdirectory(lib/common)
add_subdirectory(lib/config)
add_subdirectory(lib/utils)
add_subdirectory(lib/logger)
add_subdirectory(lib/io)
add_subdirectory(lib/tcp)
add_subdirectory(lib/http)
add_subdirectory(lib/core)

add_subdirectory(tests)

add_executable(${PROJECT_NAME}
    src/main.cc
    src/server.cc
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    webserv::core
    webserv::config
    webserv::tcp
    webserv::http
    webserv::logger
)
target_precompile_headers(${PROJECT_NAME} PRIVATE "lib/logger/include/logger.hpp")
