#include "../libft.h"

// ft_tolower()
// converts an upper-case letter to the corresponding lower-case letter.

int ft_tolower(int c)
{
    return (ft_IS_UPPER(c) ? c + 32 : c);
}