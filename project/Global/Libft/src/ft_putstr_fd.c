#include "../libft.h"

// ft_putstr_fd()
// Outputs the string ’s’ to the given file descriptor.

void ft_putstr_fd(const char* s, int fd)
{
    if (!s) {
        return;
    }
    write(fd, s, ft_strlen(s));
}