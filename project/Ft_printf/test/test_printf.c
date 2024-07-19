#include "../ft_printf.h"
#include <limits.h>
#include <stdio.h>

int main()
{
    printf("hello,%s\n", "world");
    // printf("hello,%3d\n", 32);
    // ft_printf("hello,%3d\n", 32);
    // printf("hello,%-3dagain\n", 32);
    // ft_printf("hello,%-3dagain\n", 32);
    // printf("hello,%03d\n", 5);
    // ft_printf("hello,%03d\n", 5);

    // printf("hello,%x\n", 0x1234);
    // ft_printf("hello,%x\n", 0x1234);

    // printf("hello,%#5x\n", 0x1234);
    // ft_printf("hello,%#5x\n", 0x1234);

    // printf("hello,%#8x\n", 0x1234);
    // ft_printf("hello,%#8x\n", 0x1234);

    // printf("hello,%10c\n", 'a');
    // ft_printf("hello,%10c\n", 'a');

    // printf("%7s\n", "a4564654655465");
    // ft_printf("%7s\n", "a");

    // ft_printf("%30p\n", (void*)0x7ffdd4e7b91f);
    // ft_printf("%9x\n", (unsigned int)3735929054);
    // ft_printf("%.4x\n", 11);
    // ft_printf("%.5d\n", -1234);
    // ft_printf("%-10.5d\n", 2147483647);
    // ft_printf("%.0d\n", 0);
    // ft_printf("%-1.0d5\n", -10);
    // ft_printf("%-1.0x6\n", -10);
    // ft_printf("%d", 0);
    // ft_printf("%d %d\n", 2147483647, (int)-2147483648);
    // ft_printf("%p\n", (void*)0x7fffd8dbba44);
    // ft_printf("%14p is the address", (void*)0x7ffe6b8e60c9);
    // printf("%01.d\n", 0);
    ft_printf("%01.d\n", 0);

    return 0;
}