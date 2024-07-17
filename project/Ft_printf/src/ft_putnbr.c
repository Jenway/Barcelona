#include "../ft_printf.h"

#define INT_MIN -2147483648

int ft_putnbr(int n)
{
    int count = 0;
    if (n == INT_MIN) {
        count += ft_putstr("-2147483648");
        return count;
    }
    if (n < 0) {
        count += ft_putchar('-');
        n = -n;
    }
    if (n >= 10) {
        count += ft_putnbr(n / 10);
    }
    count += ft_putchar(n % 10 + '0');
    return count;
}