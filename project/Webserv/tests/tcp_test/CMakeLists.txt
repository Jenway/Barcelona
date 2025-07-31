# in tests/tcp_test/CMakeLists.txt

add_executable(tcp_test tcp_test.cc)

# 链接到我们要测试的 tcp_lib 和 gtest
target_link_libraries(tcp_test PRIVATE
    tcp_lib
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tcp_test)
