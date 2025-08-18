cmake_minimum_required(VERSION 3.20)
project(miniRT LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(OpenCV_ROOT "${VCPKG_INSTALLED_DIR}/x64-linux/share/opencv4")
find_package(Eigen3 CONFIG REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core highgui imgcodecs)
find_package(TBB REQUIRED)
find_package(OpenMP)

add_subdirectory(lib/core)
add_subdirectory(lib/hittables)
add_subdirectory(lib/parser)
add_subdirectory(lib/render)

add_executable(miniRT
    src/main.cpp
    src/image_utils.cpp
)

target_link_libraries(miniRT PRIVATE
    core
    hittables
    parser
    render
)
find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(miniRT
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

target_compile_options(miniRT PRIVATE
    $<$<CONFIG:Release>:-O3 -march=native -funroll-loops -flto -DNDEBUG>
    $<$<CONFIG:Debug>:-O0 -g -Wall -Wextra -Wpedantic>
)

if(OpenMP_CXX_FOUND)
    target_link_libraries(miniRT PRIVATE OpenMP::OpenMP_CXX)
    message(STATUS "OpenMP found and enabled")
endif()

add_subdirectory(tests)
