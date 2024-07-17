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