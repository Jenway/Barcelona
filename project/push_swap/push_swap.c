#include "stack.h"

int main(int argc, char** argv)
{
    t_stack a;
    t_stack b;

    if (argc < 2)
        return (1);
    init_stack(&a, 'a');
    init_stack(&b, 'b');
    parse_args_to_stack(&a, argc, argv);
    sort_stack(&a, &b);
    free_stack(&a);
    free_stack(&b);
    return (0);
}
