# /tests/logger_test/CMakeLists.txt

add_executable(logger_test logger_test.cc)

target_link_libraries(logger_test PRIVATE
    logger
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(logger_test)

add_executable(logger_output_test logger_output_test.cc)

target_link_libraries(logger_output_test PRIVATE
    logger
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
)
gtest_discover_tests(logger_output_test)
