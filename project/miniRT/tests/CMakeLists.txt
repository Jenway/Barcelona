cmake_minimum_required(VERSION 3.20) # 与主 CMakeLists.txt 保持一致
project(miniRT_tests)

find_package(GTest REQUIRED)

# 创建测试可执行文件
add_executable(run_parser_tests
    test_parser.cc
)

# 链接 GTest 和您的代码。直接使用主项目定义的库目标名称。
target_link_libraries(run_parser_tests
    PRIVATE
    GTest::GTest
    GTest::Main
    core
    hittables
    parser
)

# 包含头文件目录
target_include_directories(run_parser_tests
    PRIVATE
    ${GTEST_INCLUDE_DIRS}
)
