# /tests/http_test/CMakeLists.txt

file(GLOB_RECURSE TEST_HTTP_SOURCES "*.cc")

add_executable(http_test
    ${TEST_HTTP_SOURCES}
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
