#include "../libft.h"

// ft_strjoin()
// Allocates (with malloc(3)) and returns a new string, which is the result of the concatenation of ’s1’ and ’s2’.

char* ft_strjoin(char const* s1, char const* s2)
{
    if (!s1 && !s2) {
        return (ft_strdup(""));
    }
    if (s1 && !s2) {
        return (ft_strdup(s1));
    }
    if (!s1 && s2) {
        return (ft_strdup(s2));
    }

    size_t len1 = ft_strlen(s1), len2 = ft_strlen(s2);
    char* joined = (char*)malloc(len1 + len2 + 1);
    if (!joined) {
        return NULL;
    }
    ft_strlcpy(joined, s1, len1 + 1);
    ft_strlcat(joined, s2, len1 + len2 + 1);
    return joined;
}