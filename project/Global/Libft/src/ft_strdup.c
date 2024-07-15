#include "../libft.h"

// ft_strdup()
// allocates sufficient memory for a copy of the string s1, does the copy, and returns a pointer to it.

char* ft_strdup(const char* s1)
{
    size_t len = ft_strlen(s1);
    char* dup = (char*)malloc(len + 1);
    if (dup) {
        ft_memcpy(dup, s1, len);
        dup[len] = '\0';
    }
    return dup;
}