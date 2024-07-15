#include "../libft.h"

// ft_lstnew()
// Allocates (with malloc(3)) and returns a new element.
// The variable ’content’ is initialized with the value of the parameter ’content’.
// The variable ’next’ is initialized to NULL.

t_list* ft_lstnew(void* content)
{
    t_list* newNode = (t_list*)malloc(sizeof(t_list));
    if (!newNode) {
        return NULL;
    }
    newNode->content = content;
    newNode->next = NULL;
    return newNode;
}