# in tests/tcp_test/CMakeLists.txt

add_executable(tcp_test tcp_test.cc tcp_io_test.cc tcp_acceptor.cc)

target_link_libraries(tcp_test PRIVATE
    tcp_lib
    core_lib
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
)
include(GoogleTest)
target_include_directories(tcp_test
    PRIVATE
    core_lib
)
gtest_discover_tests(tcp_test)
