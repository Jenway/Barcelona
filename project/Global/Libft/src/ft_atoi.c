#include "../libft.h"

// ft_atoi()
// converts the initial portion of the string pointed to by str to int representation.

int ft_atoi(const char* str)
{
    int sign = 1;
    int result = 0;
    while (ft_IS_SPACE(*str)) {
        ++str;
    }
    if (*str == '+' || *str == '-') {
        sign = (*str == '-' ? -1 : 1);
        ++str;
    }
    while (ft_isdigit(*str)) {
        result = result * 10 + (*str - '0');
        ++str;
    }
    return (result * sign);
}