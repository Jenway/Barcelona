#include "../ft_printf.h"

// ft_puthex - print a number in hexadecimal format
// @n: the number to print
// @uppercase: if 1, use uppercase letters
// Return: the number of characters printed

int ft_puthex(unsigned int n, int uppercase)
{
    const char* base = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    char buffer[8 * sizeof(n) + 1];
    char* ptr = buffer + sizeof(buffer) - 1;
    *ptr = '\0';
    do {
        *--ptr = base[n % 16];
        n /= 16;
    } while (n);
    return ft_putstr(ptr);
}