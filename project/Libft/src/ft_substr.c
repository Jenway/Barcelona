#include "../libft.h"

// ft_substr()
// Allocates (with malloc(3)) and returns a substring from the string ’s’. The substring begins at index ’start’ and is of maximum size ’len’.

char* ft_substr(char const* s, unsigned int start,
    size_t len)
{
    if (!s) {
        return NULL;
    }
    size_t s_len = ft_strlen(s);
    if (start > s_len) {
        return ft_strdup("");
    }
    if (len > s_len - start) {
        len = s_len - start;
    }
    char* substr = (char*)malloc(len + 1);
    if (!substr) {
        return NULL;
    }
    size_t i = 0;
    while (i < len && s[start + i]) {
        substr[i] = s[start + i];
        i++;
    }
    substr[i] = '\0';
    return substr;
}