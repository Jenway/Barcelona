#include "../libft.h"

//  ft_strlen()
// computes the length of the string s.

size_t ft_strlen(const char* s)
{
    size_t i = 0;
    while (s[i] != '\0') {
        i++;
    }
    return i;
}