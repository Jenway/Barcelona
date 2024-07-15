#include <ctype.h> // 用于比较结果的标准库函数
#include <stdio.h>

void test_isalpha(int (*func)(int))
{
    int failed = 0;
    char test_chars[] = "aZ0@"; // 测试字符：小写字母、大写字母、数字、特殊字符
    printf("Running tests for ft_isalpha...\n");
    for (size_t i = 0; i < sizeof(test_chars) - 1; ++i) {
        int expected = isalpha(test_chars[i]);
        int result = func(test_chars[i]);
        if ((expected && result) || (!expected && !result)) {
            // printf("Test passed for: %c\n", test_chars[i]);
        } else {
            printf("Test failed for: %c\n", test_chars[i]);
            failed += 1;
        }
    }
    if (failed == 0) {
        printf("All tests passed for ft_isalpha!\n");
    }
}
void test_isdigit(int (*func)(int))
{
    int failed = 0;
    char test_chars[] = "aZ09@"; // 测试字符：小写字母、大写字母、数字、特殊字符
    printf("Running tests for ft_isdigit...\n");
    for (size_t i = 0; i < sizeof(test_chars) - 1; ++i) {
        int expected = isdigit(test_chars[i]);
        int result = func(test_chars[i]);
        if ((expected && result) || (!expected && !result)) {
            // printf("Test passed for: %c\n", test_chars[i]);
        } else {
            printf("Test failed for: %c\n", test_chars[i]);
            failed += 1;
        }
    }
    if (failed == 0) {
        printf("All tests passed for ft_isdigit!\n");
    }
}

void test_isalnum(int (*func)(int))
{
    int failed = 0;
    char test_chars[] = "aZ09@"; // 测试字符：小写字母、大写字母、数字、特殊字符
    printf("Running tests for ft_isalnum...\n");
    for (size_t i = 0; i < sizeof(test_chars) - 1; ++i) {
        int expected = isalnum(test_chars[i]);
        int result = func(test_chars[i]);
        if ((expected && result) || (!expected && !result)) {
            // printf("Test passed for: %c\n", test_chars[i]);
        } else {
            printf("Test failed for: %c\n", test_chars[i]);
            failed += 1;
        }
    }
    if (failed == 0) {
        printf("All tests passed for ft_isalnum!\n");
    }
}

void test_isascii(int (*func)(int))
{
    int failed = 0;
    char test_chars[] = "aZ09@"; // 测试字符：小写字母、大写字母、数字、特殊字符
    printf("Running tests for ft_isascii...\n");
    for (size_t i = 0; i < sizeof(test_chars) - 1; ++i) {
        int expected = isascii(test_chars[i]);
        int result = func(test_chars[i]);
        if ((expected && result) || (!expected && !result)) {
            // printf("Test passed for: %c\n", test_chars[i]);
        } else {
            printf("Test failed for: %c\n", test_chars[i]);
            failed += 1;
        }
    }
    if (failed == 0) {
        printf("All tests passed for ft_isascii!\n");
    }
}

void test_isprint(int (*func)(int))
{
    int failed = 0;
    char test_chars[] = "aZ09@"; // 测试字符：小写字母、大写字母、数字、特殊字符
    printf("Running tests for ft_isprint...\n");
    for (size_t i = 0; i < sizeof(test_chars) - 1; ++i) {
        int expected = isprint(test_chars[i]);
        int result = func(test_chars[i]);
        if ((expected && result) || (!expected && !result)) {
            // printf("Test passed for: %c\n", test_chars[i]);
        } else {
            printf("Test failed for: %c\n", test_chars[i]);
            failed += 1;
        }
    }
    if (failed == 0) {
        printf("All tests passed for ft_isprint!\n");
    }
}