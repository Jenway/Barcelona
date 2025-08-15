# in tests/config_test/CMakeLists.txt

add_executable(config_test parser_size_test.cc parser_config_test.cc)

target_link_libraries(config_test PRIVATE
    webserv::config
    webserv::logger
    webserv::common
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
)
include(GoogleTest)
target_include_directories(config_test
    PRIVATE
    webserv::config
)
gtest_discover_tests(config_test)
