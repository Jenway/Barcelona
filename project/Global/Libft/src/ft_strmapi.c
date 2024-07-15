#include "../libft.h"

// ft_strmapi()
// Applies the function ’f’ to each character of the string ’s’ to
// create a new string (with malloc(3)) resulting from successive applications of ’f’.

char* ft_strmapi(char const* s, char (*f)(unsigned int, char))
{
    if (!s || !f) {
        return NULL;
    }
    size_t len = ft_strlen(s);
    char* new_str = (char*)malloc(len + 1);
    if (!new_str) {
        return NULL;
    }
    for (size_t i = 0; i < len; i++) {
        new_str[i] = f(i, s[i]);
    }
    new_str[len] = '\0';
    return new_str;
}