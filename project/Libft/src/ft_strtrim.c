#include "../libft.h"

// ft_strtrim()
// Allocates (with malloc(3)) and returns a copy of
// ’s1’ with the characters specified in ’set’ removed
// from the beginning and the end of the string.

char* ft_strtrim(char const* s1, char const* set)
{
    if (!s1) {
        return NULL;
    }
    size_t len = ft_strlen(s1);
    size_t start = 0;
    size_t end = len;
    while (s1[start] && ft_strchr(set, s1[start])) {
        start++;
    }
    while (end > start && ft_strchr(set, s1[end - 1])) {
        end--;
    }
    return ft_substr(s1, start, end - start);
}