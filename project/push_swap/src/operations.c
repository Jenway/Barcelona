#include "../stack.h"
#if defined(OP_PRINT)
#include <stdio.h>
#endif

// sa / sb
void swap(t_stack* stack)
{
    t_node* first;
    t_node* second;

    if (stack->size < 2)
        return;
    first = pop_top(stack);
    second = pop_top(stack);
    push_top(stack, first);
    push_top(stack, second);
#if defined(OP_PRINT)
    printf("s%c\n", stack->name);
#endif
}

// pa / pb
void push(t_stack* from, t_stack* to)
{
    t_node* node;

    node = pop_top(from);
    if (node)
        push_top(to, node);
#if defined(OP_PRINT)
    printf("p%c\n", to->name);
#endif
}

// ra / rb
void rotate(t_stack* stack)
{
    t_node* node;

    if (stack->size < 2)
        return;
    node = pop_top(stack);
    push_bottom(stack, node);
#if defined(OP_PRINT)
    printf("r%c\n", stack->name);
#endif
}

// rra / rrb
void reverse_rotate(t_stack* stack)
{
    t_node* node;

    if (stack->size < 2)
        return;
    node = pop_bottom(stack);
    push_top(stack, node);
#if defined(OP_PRINT)
    printf("rr%c\n", stack->name);
#endif
}
