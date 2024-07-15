#include "../libft.h"

// ft_lstclear()
// Deletes and frees the given element and every successor of that element, using the function ’del’ and free(3).

void ft_lstclear(t_list** lst, void (*del)(void*))
{

    if (!lst || !del) {
        return;
    }
    for (t_list* next; *lst; *lst = next) {
        next = (*lst)->next;
        ft_lstdelone(*lst, del);
    }
}