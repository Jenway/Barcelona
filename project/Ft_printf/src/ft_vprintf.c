#include "../ft_printf.h"

typedef int (*format_handler_t)(va_list);

typedef struct format_specifier {
    char specifier;
    format_handler_t handler;
} format_specifier_t;

static int handle_char(va_list args) { return ft_putchar(va_arg(args, int)); }

static int handle_hex_lower(va_list args) { return ft_puthex(va_arg(args, unsigned int), 0); }

static int handle_hex_upper(va_list args) { return ft_puthex(va_arg(args, unsigned int), 1); }

static int handle_pointer(va_list args) { return ft_putptr(va_arg(args, void*)); }

static int handle_unsigned(va_list args) { return ft_putunbr(va_arg(args, unsigned int)); }

static int handle_int(va_list args) { return ft_putnbr(va_arg(args, int)); }

static int handle_string(va_list args) { return ft_putstr(va_arg(args, char*)); }

static int handle_percent(va_list args)
{
    (void)args; /* 显式标记为未使用*/
    return ft_putchar('%');
}

static format_specifier_t format_handlers[] = {
    { 'c', handle_char },
    { 'x', handle_hex_lower },
    { 'X', handle_hex_upper },
    { 'p', handle_pointer },
    { 'u', handle_unsigned },
    { 'd', handle_int },
    { 'i', handle_int },
    { 's', handle_string },
    { '%', handle_percent },
};

static int format_cnt = sizeof(format_handlers) / sizeof(format_specifier_t);

int ft_vprintf(const char* format, va_list args)
{

    int result = 0;
    for (; *format; format++) {
        if (*format == '%') {
            format++;
            for (int i = 0; i < format_cnt; i++) {
                if (format_handlers[i].specifier == *format) {
                    result += format_handlers[i].handler(args);
                    break;
                }
            }
        } else {
            result += ft_putchar(*format);
        }
    }
    return result;
}
