#include "../libft.h"

//  ft_isascii()
// checks whether c is a 7-bit unsigned char value that fits into the ASCII character set.

int ft_isascii(int c)
{
    return (0 <= c && c <= 127);
}