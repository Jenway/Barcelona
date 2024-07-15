#include "../libft.h"

// ft_memchr()
// scans the initial n bytes of the memory area pointed to by s for the first instance of c.

void* ft_memchr(const void* s, int c, size_t n)
{
    unsigned char* p = (unsigned char*)s;
    for (; n; --n, ++p) {
        if (*p == (unsigned char)c) {
            return (void*)p;
        }
    }
    return NULL;
}