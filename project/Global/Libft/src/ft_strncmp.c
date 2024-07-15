#include "../libft.h"

// ft_strncmp()
// compares the first n bytes of the strings s1 and s2.

int ft_strncmp(const char* s1, const char* s2, size_t n)
{
    for (; n && *s1 && *s2; --n, ++s1, ++s2) {
        if (*s1 != *s2) {
            return (unsigned char)*s1 - (unsigned char)*s2;
        }
    }
    return n ? (unsigned char)*s1 - (unsigned char)*s2 : 0;
}