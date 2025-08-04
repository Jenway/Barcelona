# /tests/http_test/CMakeLists.txt

add_executable(http_test
    handler_test.cc
    parser_test.cc
    writer_test.cc
)

target_link_libraries(http_test PRIVATE
    webserv::logger
    webserv::io
    webserv::http
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
)

include(GoogleTest)
gtest_discover_tests(http_test)
