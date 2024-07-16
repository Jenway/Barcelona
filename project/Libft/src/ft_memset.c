#include "../libft.h"

// ft_memset()
// fills the first n bytes of the memory area pointed to by s with the constant byte c.

void* ft_memset(void* s, int c, size_t n)
{
    unsigned char* p = (unsigned char*)s;
    unsigned char uc = (unsigned char)c;
    while (n--) {
        *p++ = uc;
    }
    return s;
}