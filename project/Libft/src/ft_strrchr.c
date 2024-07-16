#include "../libft.h"

// ft_strrchr()
// returns a pointer to the last occurrence of the character c in the string s.

char* ft_strrchr(const char* s, int c)
{
    char* last = NULL;
    for (; *s; ++s) {
        if (*s == c) {
            last = (char*)s;
        }
    }
    // Consider that the character to find is '\0'.
    return (c == '\0' ? (char*)s : last);
}