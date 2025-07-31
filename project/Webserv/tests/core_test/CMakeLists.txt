# /tests/core_test/CMakeLists.txt

add_executable(connection_test connection_test.cc)

target_link_libraries(connection_test PRIVATE
    core_lib
    logger
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
)

target_include_directories(connection_test
    PRIVATE
    core_lib
)

include(GoogleTest)
gtest_discover_tests(connection_test)
