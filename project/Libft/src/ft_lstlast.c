#include "../libft.h"

// ft_lstlast()
// Returns the last element of the list.

t_list* ft_lstlast(t_list* lst)
{
    if (!lst) {
        return NULL;
    }
    while (lst->next) {
        lst = lst->next;
    }
    return lst;
}