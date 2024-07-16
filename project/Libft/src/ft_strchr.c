#include "../libft.h"

// ft_strchr()
// returns a pointer to the first occurrence of the character c in the string s.

char* ft_strchr(const char* s, int c)
{
    for (; *s; ++s) {
        if (*s == c) {
            return (char*)s;
        }
    }
    // Consider that the character to find is '\0'.
    return (c == '\0' ? (char*)s : NULL);
}