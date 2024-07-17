#include "../ft_printf.h"
#include <limits.h>
#include <stdio.h>

int main()
{
    printf("Hello, %s!\n", "world");
    ft_printf("Hello, %s!\n", "world");
    ft_printf(" NULL %s NULL \n", (char*)NULL);
    ft_printf(" %d \n", INT_MIN);
    ft_printf(" %p %p \n", LONG_MIN, LONG_MAX);
    printf(" %p %p \n", (void*)LONG_MIN, (void*)LONG_MAX);

    ft_printf(" %p %p \n", (-1), (16));
    printf(" %p %p \n", (void*)(-1), (void*)(16));

    ft_printf(" %p %p \n", (17), 15);
    printf(" %p %p \n", (void*)(17), (void*)(15));

    ft_printf(" %p %p \n", 0, LONG_MAX);
    printf(" %p %p \n", (void*)0, (void*)LONG_MAX);

    ft_printf(" hello %s %d %u %x %X with this\n", "world", 42, 42, 42, 42);
    return 0;
}