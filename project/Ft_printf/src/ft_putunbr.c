#include "../ft_printf.h"

// ft_putunbr - print an unsigned number
// @n: the number to print
// Return: the number of characters printed

int ft_putunbr(unsigned int n)
{
    char buffer[10];
    char* ptr = buffer + sizeof(buffer) - 1;
    *ptr = '\0';
    do {
        *--ptr = '0' + n % 10;
        n /= 10;
    } while (n);
    return ft_putstr(ptr);
}