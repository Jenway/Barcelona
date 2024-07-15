#include "../libft.h"

// ft_strnstr()
// locates the first occurrence of the null-terminated string needle in the string haystack, where not more than len characters are searched.

char* ft_strnstr(const char* haystack, const char* needle, size_t len)
{
    if (len == 0) {
        return (char*)haystack;
    }
    size_t needle_len = ft_strlen(needle);
    if (!needle_len) {
        return (char*)haystack;
    }
    for (; *haystack && len >= needle_len; ++haystack, --len) {
        if (*haystack == *needle && !ft_memcmp(haystack, needle, needle_len)) {
            return (char*)haystack;
        }
    }
    return NULL;
}