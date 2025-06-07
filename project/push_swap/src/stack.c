#include "../stack.h"
#include <ft_printf.h>
#include <stdlib.h>

// 初始化一个栈
void init_stack(t_stack* stack, char name)
{
    stack->top = NULL;
    stack->bottom = NULL;
    stack->size = 0;
    stack->name = name;
}

// 创建一个新节点
t_node* new_node(int value)
{
    t_node* node = malloc(sizeof(t_node));
    if (!node)
        print_error_and_exit();
    node->value = value;
    node->prev = NULL;
    node->next = NULL;
    return (node);
}

// 入栈（插入到 top）
void push_top(t_stack* stack, t_node* node)
{
    if (!node)
        return;
    node->prev = NULL;
    node->next = stack->top;
    if (stack->top)
        stack->top->prev = node;
    else
        stack->bottom = node;
    stack->top = node;
    stack->size++;
}

void push_bottom(t_stack* stack, t_node* node)
{
    if (!node)
        return;
    node->next = NULL;
    node->prev = stack->bottom;
    if (stack->bottom)
        stack->bottom->next = node;
    else
        stack->top = node;
    stack->bottom = node;
    stack->size++;
}

// 出栈（移除 top 元素）
t_node* pop_top(t_stack* stack)
{
    t_node* node;

    if (stack->size == 0)
        return (NULL);
    node = stack->top;
    stack->top = node->next;
    if (stack->top)
        stack->top->prev = NULL;
    else
        stack->bottom = NULL;
    node->next = NULL;
    node->prev = NULL;
    stack->size--;
    return (node);
}

// 出栈（移除 bottom 元素）
t_node* pop_bottom(t_stack* stack)
{
    t_node* node;

    if (stack->size == 0)
        return (NULL);
    node = stack->bottom;
    stack->bottom = node->prev;
    if (stack->bottom)
        stack->bottom->next = NULL;
    else
        stack->top = NULL;
    node->next = NULL;
    node->prev = NULL;
    stack->size--;
    return (node);
}

bool is_sorted(t_stack* stack)
{
    t_node* curr;

    if (!stack || stack->size < 2)
        return (true);
    curr = stack->top;
    while (curr->next) {
        if (curr->value > curr->next->value)
            return (false);
        curr = curr->next;
    }
    return (true);
}

void print_stack(t_stack* stack)
{
    t_node* curr = stack->top;
    while (curr) {
        ft_printf("%d ", curr->value);
        curr = curr->next;
    }
    ft_printf("\n");
}

void free_stack(t_stack* stack)
{
    t_node* tmp;

    while (stack->top) {
        tmp = stack->top->next;
        free(stack->top);
        stack->top = tmp;
    }
    stack->bottom = NULL;
    stack->size = 0;
}
