#!/bin/bash

# 编译项目
make re > /dev/null
echo "=== ScalarConverter Tests ==="

test_case() {
    echo
    echo "\$ ./convert \"$1\""
    ./convert "$1"
}

# 测试用例
test_case "a"
test_case "0"
test_case "42"
test_case "127"
test_case "-128"
test_case "nan"
test_case "nanf"
test_case "+inf"
test_case "-inf"
test_case "+inff"
test_case "-inff"
test_case "4.2"
test_case "4.2f"
test_case "42.0"
test_case "42.0f"
test_case "2147483647"     # int max
test_case "2147483648"     # overflow
test_case "9999999999999999999999999999999999" # way too big
test_case "abc"
test_case ""

# 清理
echo
echo "=== Done ==="

