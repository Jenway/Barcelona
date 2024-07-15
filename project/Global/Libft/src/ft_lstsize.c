#include "../libft.h"

// ft_lstsize()
// Counts the number of elements in a list.

int ft_lstsize(t_list* lst)
{
    int count = 0;
    while (lst) {
        count++;
        lst = lst->next;
    }
    return count;
}