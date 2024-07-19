#include "../ft_printf.h"

int ft_putstr(const char* str)
{
    static const char* null_str = "(null)";
    if (!str) {
        write(STDOUT_FILENO, null_str, 6);
        return 6;
    }
    int len = 0;
    while (str[len]) {
        len++;
    }
    write(STDOUT_FILENO, str, len);
    return len;
}

int ft_putnstr(const char* str, int n)
{
    static const char* null_str = "(null)";
    if (!str) {
        int len = (n < 6) ? n : 6;
        write(STDOUT_FILENO, null_str, len);
        return 6;
    }
    int len = 0;
    while (str[len] && len < n) {
        len++;
    }
    write(STDOUT_FILENO, str, len);
    return len;
}