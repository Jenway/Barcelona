#include "../ft_printf.h"

// ft_memset
void* ft_memset(void* s, int c, size_t n)
{
    unsigned char* p = (unsigned char*)s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
}