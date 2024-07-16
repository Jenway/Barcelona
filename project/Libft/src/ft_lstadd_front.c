#include "../libft.h"

// ft_lstadd_front()
// Adds the element ’newNode’ at the beginning of the list.

void ft_lstadd_front(t_list** lst, t_list* newNode)
{
    if (!newNode) {
        return;
    }
    newNode->next = *lst;
    *lst = newNode;
}