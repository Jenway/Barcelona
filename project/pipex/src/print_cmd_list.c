#include "../pipex.h"
#include <ft_printf.h>

void print_cmd_list(t_cmd* cmd_list)
{
    int i = 0;
    while (cmd_list) {
        ft_printf("=== Command %d ===\n", i);
        ft_printf("Path: %s\n", cmd_list->path ? cmd_list->path : "(not found)");

        ft_printf("Args: ");
        if (cmd_list->argv) {
            for (int j = 0; cmd_list->argv[j]; j++)
                ft_printf("\"%s\" ", cmd_list->argv[j]);
        } else {
            ft_printf("(null)");
        }
        ft_printf("\n\n");

        cmd_list = cmd_list->next;
        i++;
    }
}
