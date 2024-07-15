#include "../libft.h"

// ft_toupper()
// converts a lower-case letter to the corresponding upper-case letter.

int ft_toupper(int c)
{
    return (ft_IS_LOWER(c) ? c - 32 : c);
}