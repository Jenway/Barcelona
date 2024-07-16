
#include "../libft.h"

// ft_isalpha()
// checks for an alphabetic character; in the standard "C" locale, it is equivalent to (isupper(c) || islower(c)).
// In some locales, there may be additional characters  for which isalpha() is true—letters which are neither uppercase nor lower‐case.

int ft_isalpha(int c)
{
    return (ft_IS_UPPER(c) || ft_IS_LOWER(c));
}
