#include "../ft_printf.h"

static int ft_strlen(const char* str);

int print_formatted_string(const char* str, unsigned int flags, unsigned int width, unsigned int precision)
{
    char* pad = NULL;
    int if_null = 0;
    int result = 0;

    if (!str) {
        str = "(null)";
        if_null = 1;
    }

    unsigned int len = ft_strlen(str);
    unsigned int pad_len = width > len ? width - len : 0;

    if (flags & FLAGS_PRECISION) {
        len = precision < len ? precision : len;
        pad_len = width > len ? width - len : 0;
        if (if_null && precision < 6) {
            return 0;
        }
    }
    if (pad_len) {
        pad = (char*)malloc(pad_len);
        ft_memset(pad, (flags & FLAGS_ZEROPAD) ? '0' : ' ', pad_len);
        result += pad_len;
    }
    if ((flags & FLAGS_LEFT) == FLAGS_LEFT) {
        write(STDOUT_FILENO, pad, pad_len);
        result += ft_putnstr(str, len);
    } else {
        result += ft_putnstr(str, len);
        write(STDOUT_FILENO, pad, pad_len);
    }
    if (pad) {
        free(pad);
    }
    return result;
}

static int ft_strlen(const char* str)
{
    int len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}