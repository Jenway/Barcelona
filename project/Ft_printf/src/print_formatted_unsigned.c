#include "../ft_printf.h"

static int ft_numlen_unsigned(unsigned long n, unsigned long base)
{
    int len = 1;
    while (n /= base) {
        len++;
    }
    return len;
}

int print_formatted_unsigned(
    unsigned long n,
    unsigned int flags,
    unsigned int width,
    unsigned int precision,
    int base, // base for conversion
    int uppercase, // uppercase for hex
    /*
     * sign 和 prefix 所占用的长度会在父级函数中计算，这里只需要输出即可，不需要考虑
     */
    const char* prefix // prefix for hex and pointer and sign for plus and space and negative
)
{

    int result = 0;
    unsigned int num_len = ft_numlen_unsigned(n, base);
    if (flags & FLAGS_PRECISION && precision > num_len) {
        num_len = precision;
    }
    if ((flags & FLAGS_PRECISION) && precision == 0 && n == 0) {
        num_len = 0;
    }
    if (num_len < width) {
        width -= num_len;
    } else {
        width = 0;
    }

    char pad_char = (flags & FLAGS_ZEROPAD) ? '0' : ' ';

    if (prefix && (flags & FLAGS_ZEROPAD)) {
        result += ft_putstr(prefix);
    }
    if ((flags & FLAGS_LEFT) == FLAGS_LEFT) {
        for (unsigned i = 0; i < width; i++) {
            result += ft_putchar(pad_char);
        }
    }
    if (prefix && !(flags & FLAGS_ZEROPAD)) {
        result += ft_putstr(prefix);
    }

    result += ft_putnunbr(n, base, uppercase, num_len); // 输出整数

    if ((flags & FLAGS_LEFT) == 0) {
        for (unsigned i = 0; i < width; i++) {
            result += ft_putchar(pad_char);
        }
    }
    return result;
}
