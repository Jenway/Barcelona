#include "../ft_printf.h"

int ft_printf(const char* format, ...)
{
    va_list args;
    // va_start 宏的第二个参数用于定位可变参数列表的起始位置
    va_start(args, format);
    int result = ft_vprintf(format, args);
    va_end(args);
    return result;
}