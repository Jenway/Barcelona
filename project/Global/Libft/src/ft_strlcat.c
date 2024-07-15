#include "../libft.h"

//  ft_strlcat()
// The ft_strlcat() function appends the NUL-terminated string src to the end of dst. It will append at most size - strlen(dst) - 1 bytes, NUL-terminating the result.

size_t ft_strlcat(char* dst, const char* src, size_t size)
{
    if (size == 0) {
        return ft_strlen(src);
    }

    size_t i;
    size_t j;

    i = 0;
    j = 0;
    while (dst[i] && i < size) {
        i++;
    }
    while (src[j] && i + j + 1 < size) {
        dst[i + j] = src[j];
        j++;
    }
    if (i < size) {
        dst[i + j] = '\0';
    }
    while (src[j]) {
        j++;
    }
    return i + j;
}