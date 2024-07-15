#include "../libft.h"

//  ft_bzero()
// The ft_bzero() function erases the data in the n bytes of the memorystarting at the location pointed to by s,
// by writing zeros (bytes containing '\0') to that area.

void ft_bzero(void* s, size_t n)
{
    ft_memset(s, 0, n);
}