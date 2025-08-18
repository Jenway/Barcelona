add_library(parser STATIC
    parser.cpp
)

target_include_directories(parser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(parser PUBLIC
    core
    hittables
)

target_compile_features(parser PUBLIC cxx_std_20)
