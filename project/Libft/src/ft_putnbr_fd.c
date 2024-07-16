#include "../libft.h"

// ft_putnbr_fd()
// Outputs the integer ’n’ to the given file descriptor.
#define INT_MIN -2147483648
// #define INT_MIN_STR "-2147483648"

void ft_putnbr_fd(int n, int fd)
{
    const char* INT_MIN_STR = "-2147483648";
    if (n == INT_MIN) {
        ft_putstr_fd(INT_MIN_STR, fd);
        return;
    }
    if (n < 0) {
        ft_putchar_fd('-', fd);
        n = -n;
    }
    if (n > 9) {
        ft_putnbr_fd(n / 10, fd);
    }
    ft_putchar_fd(n % 10 + '0', fd);
}