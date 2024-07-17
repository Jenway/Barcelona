#if !defined(__FT_PRINTF_H__)
#define __FT_PRINTF_H__

#include <stdarg.h> // va_list, va_start, va_arg, va_end
#include <unistd.h>

int ft_printf(const char* format, ...);
int ft_vprintf(const char* format, va_list args);

int ft_putchar(char c);
int ft_putnbr(int n);
int ft_putstr(const char* s);
int ft_puthex(unsigned int n, int uppercase);
int ft_putunbr(unsigned int n);
int ft_putptr(void* ptr);

#endif