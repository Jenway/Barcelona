#include "../pipex.h"
#include <libft.h>
#include <stdlib.h>

// 创建新命令节点
static t_cmd* create_cmd_node(char* cmd_str, char** envp)
{
    t_cmd* cmd = malloc(sizeof(t_cmd));
    if (!cmd)
        return NULL;

    cmd->argv = ft_split(cmd_str, ' ');
    if (!cmd->argv || !cmd->argv[0]) {
        free(cmd);
        return NULL;
    }

    cmd->path = get_cmd_path(cmd->argv[0], envp);
    if (!cmd->path) {
        // 可执行文件不存在，仍然保留 argv，用于报错
        // execve 会失败并打印错误
    }

    cmd->next = NULL;
    return cmd;
}

// 添加命令到链表末尾
static void append_cmd(t_cmd** list, t_cmd* new_cmd)
{
    t_cmd* last;

    if (!*list) {
        *list = new_cmd;
        return;
    }
    last = *list;
    while (last->next)
        last = last->next;
    last->next = new_cmd;
}

// 释放整个命令链表
void free_cmd_list(t_cmd* cmd_list)
{
    t_cmd* tmp;

    while (cmd_list) {
        tmp = cmd_list->next;
        if (cmd_list->argv) {
            for (int i = 0; cmd_list->argv[i]; i++)
                free(cmd_list->argv[i]);
            free(cmd_list->argv);
        }
        free(cmd_list->path);
        free(cmd_list);
        cmd_list = tmp;
    }
}

// 主解析函数
t_cmd* parse_commands(int argc, char** argv, char** envp, int is_here_doc)
{
    t_cmd* cmd_list = NULL;
    t_cmd* new_cmd;
    int i = is_here_doc ? 3 : 2; // skip infile / here_doc

    while (i < argc - 1) // skip outfile
    {
        new_cmd = create_cmd_node(argv[i], envp);
        if (!new_cmd) {
            free_cmd_list(cmd_list);
            return NULL;
        }
        append_cmd(&cmd_list, new_cmd);
        i++;
    }
    return cmd_list;
}
