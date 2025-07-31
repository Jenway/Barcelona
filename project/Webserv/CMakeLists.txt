# /CMakeLists.txt

cmake_minimum_required(VERSION 3.20)
project(WebServer CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

enable_testing()

find_package(GTest REQUIRED)
find_package(fmt CONFIG REQUIRED)

add_subdirectory(lib/logger)
add_subdirectory(tests)

add_executable(${PROJECT_NAME} src/main.cc)
target_link_libraries(${PROJECT_NAME} PRIVATE logger)
target_precompile_headers(${PROJECT_NAME} PRIVATE "lib/logger/include/logger.hpp")
