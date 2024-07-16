#include "../libft.h"

// ft_lstadd_back()
// Adds the element ’newNode’ at the end of the list.

void ft_lstadd_back(t_list** lst, t_list* newNode)
{
    if (!lst || !newNode) {
        return;
    }
    if (!*lst) {
        *lst = newNode;
        return;
    }
    ft_lstlast(*lst)->next = newNode;
}