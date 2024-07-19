#include "../ft_printf.h"

static int ft_num_prec(unsigned long n, unsigned long base);
static int ft_putnnbr_helper(unsigned long n, unsigned long base, int uppercase, int len);

/*
ft_putnnbr: print a unsigned number with a specific base and precision
*/
int ft_putnunbr(unsigned long n, unsigned long base, int uppercase, int len)
{

    int count = 0;
    int prec = ft_num_prec(n, base);

    if (len > prec) {
        for (int i = 0; i < len - prec; i++) {
            count += ft_putchar('0');
        }
    } else {
        len = prec;
    }
    count += ft_putnnbr_helper(n, base, uppercase, len);
    return count;
}

static int ft_num_prec(unsigned long n, unsigned long base)
{
    int prec = 0;
    while (n) {
        prec++;
        n /= base;
    }
    return prec;
}

static int ft_putnnbr_helper(unsigned long n, unsigned long base, int uppercase, int len)
{
    char digits[][17] = {
        "0123456789abcdef",
        "0123456789ABCDEF"
    };
    int count = 0;

    if (n == 0 || len == 0) {
        return count;
    }

    if (n >= base) {
        count += ft_putnnbr_helper(n / base, base, uppercase, len - 1);
    }
    count += ft_putchar(digits[uppercase][n % base]);
    return count;
}
