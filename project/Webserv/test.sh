#!/bin/bash
#
# test.sh: 一键编译并运行所有或指定的 CTest 单元测试
#
# 用法:
#   ./test.sh         - 运行所有测试
#   ./test.sh <regex> - 只运行名称匹配 <regex> 的测试
#
# 示例:
#   ./test.sh Logger    - 只运行名字里包含 "Logger" 的测试
#   ./test.sh Tcp       - 只运行 TcpComponentsTest 套件的测试
#   ./test.sh Partial   - 只运行名字里包含 "Partial" 的测试
#
set -e # 任何命令失败则立即退出脚本

echo "--- Building project and tests... ---"
# 使用 --build build 代替 cd build && make，更通用
cmake --build build

CTEST_ARGS="--output-on-failure"
if [ "$#" -gt 0 ]; then
    # 如果有参数，就用 -R 选项来筛选测试
    echo ""
    echo "--- Running tests matching regex: $1 ---"
    CTEST_ARGS="$CTEST_ARGS -R $1"
else
    echo ""
    echo "--- Running all tests... ---"
fi

# 3. 进入构建目录并运行 CTest
cd build
ctest $CTEST_ARGS
cd .. # 返回根目录

echo ""
echo "✅ Test run finished successfully!"