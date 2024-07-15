#include "../libft.h"

//  isalnum()
// checks for an alphanumeric character; it is equivalent to (isalpha(c) ||  isdigit(c)).

int ft_isalnum(int c)
{
    return (ft_isalpha(c) || ft_isdigit(c));
}