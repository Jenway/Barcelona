#include "stack.h"
#include <fcntl.h>
#include <get_next_line.h> // 假设你有 get_next_line 实现
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void execute_op(const char* op, t_stack* a, t_stack* b)
{
    if (strcmp(op, "sa") == 0) {
        swap(a);
    } else if (strcmp(op, "sb") == 0) {
        swap(b);
    } else if (strcmp(op, "ss") == 0) {
        swap(a);
        swap(b);
    } else if (strcmp(op, "pa") == 0) {
        push(b, a);
    } else if (strcmp(op, "pb") == 0) {
        push(a, b);
    } else if (strcmp(op, "ra") == 0) {
        rotate(a);
    } else if (strcmp(op, "rb") == 0) {
        rotate(b);
    } else if (strcmp(op, "rr") == 0) {
        rotate(a);
        rotate(b);
    } else if (strcmp(op, "rra") == 0) {
        reverse_rotate(a);
    } else if (strcmp(op, "rrb") == 0) {
        reverse_rotate(b);
    } else if (strcmp(op, "rrr") == 0) {
        reverse_rotate(a);
        reverse_rotate(b);
    } else {
        write(2, "Error\n", 6);
        exit(EXIT_FAILURE);
    }
}

void read_and_execute_ops(t_stack* a, t_stack* b)
{
    char* line;

    while ((line = get_next_line(0)) != NULL) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';

        execute_op(line, a, b);

        free(line);
    }
}

int main(int argc, char** argv)
{
    t_stack a;
    t_stack b;

    if (argc < 2)
        return (1);
    init_stack(&a, 'a');
    init_stack(&b, 'b');
    parse_args_to_stack(&a, argc, argv);

    read_and_execute_ops(&a, &b);

    if (is_sorted(&a) && b.size == 0)
        write(1, "OK\n", 3);
    else
        write(1, "KO\n", 3);

    free_stack(&a);
    free_stack(&b);
    return (0);
}
