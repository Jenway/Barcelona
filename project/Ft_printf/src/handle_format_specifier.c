#include "../ft_printf.h"

static int handle_char(va_list args, unsigned int flags, unsigned int width, unsigned int precision);
static int handle_unsigned(va_list args, unsigned int flags, unsigned int width, unsigned int precision);
static int handle_int(va_list args, unsigned int flags, unsigned int width, unsigned int precision);
static int handle_hex_lower(va_list args, unsigned int flags, unsigned int width, unsigned int precision);
static int handle_hex_upper(va_list args, unsigned int flags, unsigned int width, unsigned int precision);
static int handle_pointer(va_list args, unsigned int flags, unsigned int width, unsigned int precision);
static int handle_string(va_list args, unsigned int flags, unsigned int width, unsigned int precision);
static int handle_percent(va_list args, unsigned int flags, unsigned int width, unsigned int precision);

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

int handle_format_specifier(const char* format, va_list args, unsigned int flags, unsigned int width, unsigned int precision)
{
    int result = 0;
    for (int i = 0; i < format_cnt; i++) {
        if (format_handlers[i].specifier == *format) {
            result += format_handlers[i].handler(args, flags, width, precision);
            break;
        }
    }
    return result;
}

/*
 * 因为 tester 对速度有要求，所以这里使用了动态内存分配
 */
static int handle_char(va_list args, unsigned int flags, unsigned int width, unsigned int precision)

{
    (void)precision; // 精度对字符类型无效，可以继续忽略
    char ch = (char)va_arg(args, int);
    int result = 0;
    char* str = NULL;
    if (width > 1) {
        str = (char*)malloc(width - 1);
        ft_memset(str, ' ', width - 1);
    }
    if (((flags & FLAGS_LEFT) == FLAGS_LEFT) && str) {
        result += ft_putstr(str);
        result += ft_putchar(ch);
    } else if (str) {
        result += ft_putchar(ch);
        result += ft_putstr(str);
    } else {
        result += ft_putchar(ch);
    }
    if (str) {
        free(str);
    }
    return result;
}

static int handle_int(va_list args, unsigned int flags, unsigned int width, unsigned int precision)
{
    int result = 0;
    int n = va_arg(args, int);
    int sign = 0;
    if (n < 0) {
        n = -n;
        width = width > 0 ? width - 1 : 0;
        sign = 1;
    }
    const char* prefix = NULL;
    if (flags & FLAGS_PLUS) {
        prefix = sign ? "-" : "+";
    } else if (flags & FLAGS_SPACE) {
        prefix = sign ? "-" : " ";
    } else {
        prefix = sign ? "-" : NULL;
    }
    return result + print_formatted_unsigned((unsigned int)n, flags, width, precision, 10, 0, prefix);
}

static int handle_unsigned(va_list args, unsigned int flags, unsigned int width, unsigned int precision)
{
    return print_formatted_unsigned(va_arg(args, unsigned int), flags, width, precision, 10, 0, NULL);
}

static int handle_hex_lower(va_list args, unsigned int flags, unsigned int width, unsigned int precision)
{
    unsigned int n = va_arg(args, unsigned int);
    const char* prefix = (flags & FLAGS_HASH) ? "0x" : NULL;
    if (flags & FLAGS_HASH && n == 0) {
        return print_formatted_unsigned(n, flags, width, precision, 16, 0, NULL);
    }
    width = (width > 2 && flags & FLAGS_HASH) ? width - 2 : width;
    return print_formatted_unsigned(n, flags, width, precision, 16, 0, prefix);
}

static int handle_hex_upper(va_list args, unsigned int flags, unsigned int width, unsigned int precision)
{
    unsigned int n = va_arg(args, unsigned int);

    const char* prefix = (flags & FLAGS_HASH) ? "0X" : NULL;
    if (flags & FLAGS_HASH && n == 0) {
        return print_formatted_unsigned(n, flags, width, precision, 16, 1, NULL);
    }
    width = (width > 2 && flags & FLAGS_HASH) ? width - 2 : width;
    return print_formatted_unsigned(n, flags, width, precision, 16, 1, prefix);
}

static int handle_pointer(va_list args, unsigned int flags, unsigned int width, unsigned int precision)
{
    void* ptr = va_arg(args, void*);
    if (ptr) {
        width = (width > 2) ? width - 2 : width;
        return print_formatted_unsigned((unsigned long)ptr, flags, width, precision, 16, 0, "0x");
    } else {
        return print_formatted_string("(nil)", flags, width, precision);
    }
}

static int handle_string(va_list args, unsigned int flags, unsigned int width, unsigned int precision)
{
    return print_formatted_string(va_arg(args, const char*), flags, width, precision);
}

static int handle_percent(va_list args, unsigned int flags, unsigned int width, unsigned int precision)
{
    (void)precision;
    (void)flags;
    (void)width;
    (void)args; /* 显式标记为未使用*/
    return ft_putchar('%');
}
