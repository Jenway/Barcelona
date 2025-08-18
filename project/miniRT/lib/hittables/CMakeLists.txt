add_library(hittables STATIC
    bvh.cpp
)

target_include_directories(hittables PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(hittables PUBLIC core)

target_compile_features(hittables PUBLIC cxx_std_20)
