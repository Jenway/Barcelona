# /tests/core_test/CMakeLists.txt

add_executable(core_test
    connection_test.cc
    poller_test.cc
)

target_link_libraries(core_test PRIVATE
    core_lib
    logger
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
)

target_include_directories(core_test
    PRIVATE
    core_lib
)

include(GoogleTest)
gtest_discover_tests(core_test)
