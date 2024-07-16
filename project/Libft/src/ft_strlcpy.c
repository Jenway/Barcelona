#include "../libft.h"

//  ft_strlcpy()
// copies up to size - 1 characters from the NUL-terminated string src to dst, NUL-terminating the result.

size_t ft_strlcpy(char* dst, const char* src, size_t size)
{

    if (size == 0) {
        return ft_strlen(src);
    }

    size_t i = 0;
    while (i < size - 1 && src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }

    if (size > 0) {
        dst[i] = '\0';
    }

    while (src[i] != '\0') {
        i++;
    }

    return i;
}