#include "../libft.h"

// ft_calloc()
// allocates memory for an array of nmemb elements of size bytes each and returns a pointer to the allocated memory.

void* ft_calloc(size_t nmemb, size_t size)
{
    void* ptr = malloc(nmemb * size);
    if (ptr) {
        ft_bzero(ptr, nmemb * size);
    }
    return ptr;
}