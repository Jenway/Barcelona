#include "../libft.h"

//  ft_memcpy()
// copies n bytes from memory area src to memory area dest. The memory areas must not overlap.

void* ft_memcpy(void* dest, const void* src, size_t n)
{
    if (!dest && !src) {
        return NULL;
    }
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}