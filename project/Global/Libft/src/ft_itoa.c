#include "../libft.h"

// Allocates (with malloc(3)) and returns a string
// representing the integer received as an argument.
// Negative numbers must be handled.

char* ft_itoa(int n)
{
    size_t len = 1;
    int n_cpy = n;
    while (n_cpy /= 10) {
        len++;
    }
    if (n < 0) {
        len++;
    }
    char* str = (char*)malloc(len + 1);
    if (!str) {
        return NULL;
    }
    str[len] = '\0';
    if (n < 0) {
        str[0] = '-';
    }
    if (n == 0) {
        str[0] = '0';
    }
    while (len > 0 && n) {
        str[--len] = '0' + (n < 0 ? -(n % 10) : n % 10);
        n /= 10;
    }
    return str;
}