#include "../stack.h"
#include <ctype.h>
#include <libft.h>
#include <limits.h>
#include <stdbool.h>

static bool is_valid_number(const char* s)
{
    if (!s || *s == '\0')
        return false;

    if (*s == '-' || *s == '+')
        s++;
    if (!isdigit(*s))
        return false;

    while (*s) {
        if (!isdigit(*s))
            return false;
        s++;
    }
    return true;
}

// 字符串转整数，带溢出检测
static int safe_atoi(const char* s)
{
    long long n = 0;
    int sign = 1;

    if (*s == '-' || *s == '+') {
        if (*s == '-')
            sign = -1;
        s++;
    }
    while (*s && isdigit(*s)) {
        n = n * 10 + (*s - '0');
        if (sign * n > INT_MAX || sign * n < INT_MIN)
            print_error_and_exit();
        s++;
    }
    return ((int)(sign * n));
}

static bool is_duplicate(t_stack* stack, int value)
{
    t_node* curr = stack->top;
    while (curr) {
        if (curr->value == value)
            return true;
        curr = curr->next;
    }
    return false;
}

void parse_args_to_stack(t_stack* a, int argc, char** argv)
{
    char** tokens;
    int i, j;
    int value;

    i = 1;
    while (i < argc) {
        tokens = ft_split(argv[i], ' ');
        if (!tokens)
            print_error_and_exit();
        j = 0;
        while (tokens[j]) {
            if (!is_valid_number(tokens[j]))
                print_error_and_exit();
            value = safe_atoi(tokens[j]);
            if (is_duplicate(a, value))
                print_error_and_exit();
            push_bottom(a, new_node(value));
            j++;
        }
        // free tokens
        j = 0;
        while (tokens[j])
            free(tokens[j++]);
        free(tokens);
        i++;
    }
}
