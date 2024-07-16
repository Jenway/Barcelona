#include "../libft.h"

// ft_memcmp()
// compares the first n bytes of the memory areas s1 and s2.

int ft_memcmp(const void* s1, const void* s2, size_t n)
{
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    for (; n; --n, ++p1, ++p2) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
    }
    return 0;
}