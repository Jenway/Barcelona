#include "../libft.h" // 假设你的函数在这个头文件中声明
#include <stdio.h>

extern void test_isalpha(int (*func)(int));
extern void test_isdigit(int (*func)(int));
extern void test_isalnum(int (*func)(int));
extern void test_isascii(int (*func)(int));
extern void test_isprint(int (*func)(int));

// ft_strlen.c ft_memset.c ft_bzero.c ft_memcpy.c ft_memmove.c ft_strlcpy.c ft_strlcat.c
extern void test_strlen(size_t (*func)(const char*));
extern void test_memset(void* (*func)(void*, int, size_t));
extern void test_bzero(void (*func)(void*, size_t));
extern void test_memcpy(void* (*func)(void*, const void*, size_t));
extern void test_memmove(void* (*func)(void*, const void*, size_t));
extern void test_strlcpy(size_t (*func)(char*, const char*, size_t));
extern void test_strlcat(size_t (*func)(char*, const char*, size_t));

extern void test_itoa(char * (*func)(int));

int main()
{
    // test_isalpha(&ft_isalpha);
    // test_isdigit(&ft_isdigit);
    // test_isalnum(&ft_isalnum);
    // test_isascii(&ft_isascii);
    // test_isprint(&ft_isprint);

    printf("finished testing is functions\n");
    // test_strlen(&ft_strlen);
    // test_memset(&ft_memset);
    // test_bzero(&ft_bzero);
    // test_memcpy(&ft_memcpy);
    // test_memmove(&ft_memmove);
    // test_strlcpy(&ft_strlcpy);
    // test_strlcat(&ft_strlcat);
    printf("finished testing mem string functions\n");
    test_itoa(&ft_itoa);

    return 0;
}
