#include "../ft_printf.h"

#define PTR_PREFIX "0x"
#define PTR_HEX_LENGTH 16

int ft_puthex_wlen(unsigned long n, int uppercase, int width)
{
    const char* base = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    char buffer[8 * sizeof(n) + 1];
    char* ptr = buffer + sizeof(buffer) - 1;
    *ptr = '\0';
    do {
        *--ptr = base[n % 16];
        n /= 16;
    } while (n);
    int len = sizeof(buffer) - 1 - (ptr - buffer);

    while (len < width) {
        ft_putchar('0');
        len++;
    }
    return ft_putstr(ptr);
}

int ft_putptr(void* ptr)
{
    if (!ptr) {
        return ft_putstr("(nil)");
    }
    int result = ft_putstr(PTR_PREFIX);
    if (0 == (unsigned long)ptr) {
        return result + ft_puthex_wlen((unsigned long)ptr, 0, PTR_HEX_LENGTH);
    } else {
        return result + ft_puthex_wlen((unsigned long)ptr, 0, -1);
    }
}