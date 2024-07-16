#include <stdio.h>
#include <string.h>

void test_strlen(size_t (*func)(const char*))
{
    int failed = 0;
    char* test_strings[] = {
        "Hello, world!",
        "This is a test string.",
        "Another test string.",
        "Yet another test string.",
        "The quick brown fox jumps over the lazy dog."
    };
    printf("Running tests for ft_strlen...\n");
    for (size_t i = 0; i < sizeof(test_strings) / sizeof(test_strings[0]); ++i) {
        size_t expected = strlen(test_strings[i]);
        size_t result = func(test_strings[i]);
        if (expected == result) {
            // printf("Test passed for: %s\n", test_strings[i]);
        } else {
            printf("Test failed for: %s\n", test_strings[i]);
            failed += 1;
        }
    }
    if (failed == 0) {
        printf("All tests passed for ft_strlen!\n");
    }
}

void test_memset(void* (*func)(void*, int, size_t))
{
    int failed = 0;
    char test_string[] = "Hello, world!";
    char expected[] = "AAAAAAAAAAAAA";
    printf("Running tests for ft_memset...\n");
    func(test_string, 'A', 13);
    if (memcmp(test_string, expected, 13) == 0) {
        // printf("Test passed for: %s\n", test_string);
    } else {
        printf("Test failed for: %s\n", test_string);
        failed += 1;
    }
    if (failed == 0) {
        printf("All tests passed for ft_memset!\n");
    }
}

void test_bzero(void (*func)(void*, size_t))
{
    int failed = 0;
    char test_string[] = "Hello, world!";
    char sample[] = "Hello, world!";
    char expected[] = "             ";
    printf("Running tests for ft_bzero...\n");
    func(test_string, 13);
    bzero(sample, 13);
    if (memcmp(test_string, sample, 13) == 0) {
        // printf("Test passed for: %s\n", test_string);
    } else {
        printf("Test failed for: %s\n", test_string);
        printf("Expected: %s\n", expected);
        printf("Result: %s\n", test_string);
        printf("len: %ld\n", strlen(test_string));
        printf("memcmp: %d\n", memcmp(test_string, expected, 13));
        printf("memcmp: %d\n", memcmp(test_string, expected, 13));
        failed += 1;
    }
    if (failed == 0) {
        printf("All tests passed for ft_bzero!\n");
    }
}

void test_memcpy(void* (*func)(void*, const void*, size_t))
{
    int failed = 0;
    char src[] = "Hello, world!";
    char dest[14];
    char expected[] = "Hello, world!";
    printf("Running tests for ft_memcpy...\n");
    func(dest, src, 14);
    if (memcmp(dest, expected, 14) == 0) {
        // printf("Test passed for: %s\n", dest);
    } else {
        printf("Test failed for: %s\n", dest);
        failed += 1;
    }
    if (failed == 0) {
        printf("All tests passed for ft_memcpy!\n");
    }
}

void test_memmove(void* (*func)(void*, const void*, size_t))
{
    int failed = 0;
    char src[] = "Hello, world!";
    char dest[14];
    char expected[] = "Hello, world!";
    printf("Running tests for ft_memmove...\n");
    func(dest, src, 14);
    if (memcmp(dest, expected, 14) == 0) {
        // printf("Test passed for: %s\n", dest);
    } else {
        printf("Test failed for: %s\n", dest);
        failed += 1;
    }
    if (failed == 0) {
        printf("All tests passed for ft_memmove!\n");
    }
}
void test_strlcpy(size_t (*func)(char*, const char*, size_t))
{
    int failed = 0;
    char dest[14];
    char src[] = "Hello, world!";
    char expected[] = "Hello, world!";
    printf("Running tests for ft_strlcpy...\n");
    size_t result = func(dest, src, 14);
    if (memcmp(dest, expected, 14) == 0 && result == 13) {
        // printf("Test passed for: %s\n", dest);
    } else {
        printf("Test failed for: %s\n", dest);
        failed += 1;
    }
    if (failed == 0) {
        printf("All tests passed for ft_strlcpy!\n");
    }
}

void test_strlcat(size_t (*func)(char*, const char*, size_t))
{
    int failed = 0;
    char dest[14] = "Hello, ";
    char src[] = "world!";
    char expected[] = "Hello, world!";
    printf("Running tests for ft_strlcat...\n");
    size_t result = func(dest, src, 14);

    if (memcmp(dest, expected, 14) == 0 && result == 13) {
        // printf("Test passed for: %s\n", dest);
    } else {
        printf("Expected: %s\n", expected);
        printf("Result: %s\n", dest);
        printf("Test failed for: %s\n", dest);
        failed += 1;
    }
    if (failed == 0) {
        printf("All tests passed for ft_strlcat!\n");
    }
}