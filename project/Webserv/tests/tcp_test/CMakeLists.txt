# in tests/tcp_test/CMakeLists.txt

add_executable(tcp_acceptor_test tcp_test.cc)
target_link_libraries(tcp_acceptor_test PRIVATE
    tcp_lib
    GTest::gtest
    GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(tcp_acceptor_test)

add_executable(tcp_io_test tcp_io_test.cc)
target_link_libraries(tcp_io_test PRIVATE
    tcp_lib
    core_lib
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
)
include(GoogleTest)
target_include_directories(tcp_io_test
    PRIVATE
    core_lib
)
gtest_discover_tests(tcp_io_test)
