add_library(render STATIC
    renderer.cpp
)

target_include_directories(render PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(render PUBLIC
    core
    hittables
    parser
    TBB::tbb
    ${OpenCV_LIBS}
)

if(OpenMP_CXX_FOUND)
    target_link_libraries(render PUBLIC OpenMP::OpenMP_CXX)
endif()

target_compile_features(render PUBLIC cxx_std_20)
