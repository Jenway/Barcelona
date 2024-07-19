#if !defined(__FT_PRINTF_H__)
#define __FT_PRINTF_H__

#include <stdarg.h> // va_list, va_start, va_arg, va_end
#include <stdlib.h> // malloc, free
#include <unistd.h>

int ft_printf(const char* format, ...);
int ft_vprintf(const char* format, va_list args);

int handle_format_specifier(const char* format, va_list args, unsigned int flags, unsigned int width, unsigned int precision);
int print_formatted_unsigned(unsigned long n, unsigned int flags, unsigned int width, unsigned int precision, int base, int uppercase, const char* prefix);
int print_formatted_string(const char* str, unsigned int flags, unsigned int width, unsigned int precision);

void* ft_memset(void* s, int c, size_t n);

int ft_putchar(char c);
int ft_putstr(const char* s);
int ft_putnstr(const char* str, int n);
int ft_putnunbr(unsigned long n, unsigned long base, int uppercase, int len);

// output function type
// typedef void (*put_func_type)(char character, void* buffer, size_t idx, size_t maxlen);
typedef int (*format_handler_t)(va_list, unsigned int, unsigned int, unsigned int);

typedef struct format_specifier {
    char specifier;
    format_handler_t handler;
} format_specifier_t;

// internal flag definitions
#define FLAGS_ZEROPAD (1U << 0U)
#define FLAGS_LEFT (1U << 1U)
#define FLAGS_PLUS (1U << 2U)
#define FLAGS_SPACE (1U << 3U)
#define FLAGS_HASH (1U << 4U)
#define FLAGS_PRECISION (1U << 5U)

#define DEFAULT_FLAGS (FLAGS_LEFT)
#define DEFAULT_WIDTH (0U)
#define DEFAULT_PRECISION (0U)

#endif