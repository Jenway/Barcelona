#include "../libft.h"

// ft_striteri()
// Applies the function ’f’ to each character of the string passed as argument,

void ft_striteri(char* s, void (*f)(unsigned int, char*))
{
    if (!s || !f) {
        return;
    }
    for (size_t i = 0; s[i]; i++) {
        f(i, &s[i]);
    }
}