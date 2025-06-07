#ifndef STACK_H
# define STACK_H

# include <unistd.h>
# include <stdbool.h>
# include <stdlib.h>

typedef struct s_node {
	int             value;
	struct s_node  *prev;
	struct s_node  *next;
}	t_node;

typedef struct s_stack {
	t_node	*top;      // 栈顶（即链表头）
	t_node	*bottom;   // 栈底（即链表尾）
	int		size;      // 栈大小
	char	name;      // 栈名 'a' 或 'b'
}	t_stack;

// 基础操作
void	init_stack(t_stack *stack, char name);
t_node*	new_node(int value);
void	push_top(t_stack *stack, t_node *node);
void	push_bottom(t_stack* stack, t_node* node);
t_node*	pop_top(t_stack *stack);
t_node*	pop_bottom(t_stack *stack);
void	print_stack(t_stack* stack);

// push_swap 操作
void	swap(t_stack *stack);             // sa / sb
void	push(t_stack *from, t_stack *to); // pa / pb
void	rotate(t_stack *stack);           // ra / rb
void	reverse_rotate(t_stack *stack);   // rra / rrb

// 工具函数
void	free_stack(t_stack *stack);
bool	is_sorted(t_stack *stack);

#define print_error_and_exit() \
	do { \
		write(2, "Error\n", 6); \
		exit(EXIT_FAILURE); \
	} while (0)

// 解析输入
void	parse_args_to_stack(t_stack *a, int argc, char **argv);

void sort_stack(t_stack* a, t_stack* b);
void radix_sort(t_stack* a, t_stack* b);

#endif
