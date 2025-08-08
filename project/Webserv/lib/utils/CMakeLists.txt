# in lib/utils/CMakeLists.txt

add_library(utils_lib
    src/FileUtils.cc
)

add_library(webserv::utils ALIAS utils_lib)


target_include_directories(utils_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(utils_lib PUBLIC
    webserv::common
)
