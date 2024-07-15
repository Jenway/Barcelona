#include "../libft.h"

// ft_lstmap()
// Iterates the list ’lst’ and applies the function ’f’ to the content of each element.
// Creates a new list resulting of the successive applications of the function ’f’.
//  The ’del’ function is used to delete the content of an element if needed.

t_list* ft_lstmap(t_list* lst, void* (*f)(void*), void (*del)(void*))
{
    if (!lst || !f) {
        return NULL;
    }
    t_list* new_lst = NULL;
    t_list* new_elem = NULL;
    while (lst) {
        new_elem = ft_lstnew(f(lst->content));
        if (!new_elem) {
            ft_lstclear(&new_lst, del);
            return NULL;
        }
        ft_lstadd_back(&new_lst, new_elem);
        lst = lst->next;
    }
    return new_lst;
}
