#include "../ft_printf.h"

static int ft_isdigit(int c) { return (c >= '0' && c <= '9'); }

// read a number from the format string
static int read_number(const char** format)
{
    int number = 0;
    while (ft_isdigit(**format)) {
        number = number * 10 + (**format - '0');
        (*format)++;
    }
    return number;
}

int ft_vprintf(const char* format, va_list va)
{
    int result = 0;
    unsigned int flags = DEFAULT_FLAGS,
                 width = DEFAULT_WIDTH,
                 precision = DEFAULT_PRECISION;

    for (; *format; format++) {
        if (*format == '%') {
            flags = DEFAULT_FLAGS;
            width = DEFAULT_WIDTH;
            precision = DEFAULT_PRECISION;
            format++;

            while (*format == '-' || *format == '+' || *format == ' ' || *format == '#' || *format == '0') {
                if (*format == '-') {
                    flags &= ~FLAGS_LEFT;
                } else if (*format == '+') {
                    flags |= FLAGS_PLUS;
                } else if (*format == ' ') {
                    flags |= FLAGS_SPACE;
                } else if (*format == '#') {
                    flags |= FLAGS_HASH;
                } else if (*format == '0') {
                    flags |= FLAGS_ZEROPAD;
                }
                format++;
            }

            // width
            if (*format == '*') {
                width = va_arg(va, int);
                format++;
            } else if (ft_isdigit(*format)) {
                width = read_number(&format);
            }

            // precision
            if (*format == '.') {
                flags |= FLAGS_PRECISION;
                // '0' flag is ignored when precision is present
                flags &= ~FLAGS_ZEROPAD;
                format++;
                if (*format == '*') {
                    precision = va_arg(va, int);
                    format++;
                } else {
                    precision = read_number(&format);
                }
            }

            result += handle_format_specifier(format, va, flags, width, precision);
        } else {
            result += ft_putchar(*format);
        }
    }
    return result;
}
