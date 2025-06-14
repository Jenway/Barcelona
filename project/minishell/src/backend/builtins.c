#include <ctype.h>
#include <minishell.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
- Your shell must implement the following builtins:
    - echo with option -n
    - cd with only a relative or absolute path
    - pwd with no options
    - export with no options
    - unset with no options
    - env with no options or arguments
    - exit with no options
*/

int is_builtin(const char* cmd)
{
// 定义内建命令列表的宏
#define BUILTIN_COMMANDS \
    X(echo)              \
    X(cd)                \
    X(pwd)               \
    X(export)            \
    X(unset)             \
    X(env)               \
    X(exit)

// 展开为多个if判断
#define X(name)                  \
    if (strcmp(cmd, #name) == 0) \
        return 1;
    BUILTIN_COMMANDS
#undef X // 取消X的定义避免污染

    return 0; // 不是内建命令
}

// 帮助命令
static void builtin_help()
{
    printf("\033[1;33mAvailable commands:\033[0m\n");
    printf("  \033[1mcd\033[0m [dir]       - Change directory\n");
    printf("  \033[1mexit\033[0m [code]    - Exit the shell\n");
    printf("  \033[1mexport\033[0m VAR=val - Set environment variable\n");
    printf("  \033[1munset\033[0m VAR      - Unset environment variable\n");
    printf("  \033[1menv\033[0m           - List environment variables\n");
    printf("  \033[1mpwd\033[0m           - Print working directory\n");
    printf("  \033[1mhelp\033[0m          - Show this help\n");
    printf("\n\033[1;33mFeatures:\033[0m\n");
    printf("  - Command history (↑/↓ keys)\n");
    printf("  - Auto-completion (Tab key)\n");
    printf("  - Line editing (Ctrl+A, Ctrl+E, etc.)\n");
    printf("  - Syntax highlighting\n");
}

static int echo_impl(t_cmd* cmd)
{
    char** argv = cmd->argv;
    int i = 1, newline = 1;

    // 处理 -n 选项（禁止末尾换行）
    if (argv[1] && strcmp(argv[1], "-n") == 0) {
        newline = 0;
        i++;
    }

    // 逐参数输出
    for (; argv[i]; i++) {
        printf("%s", argv[i]); // 直接输出参数（不解析 $）
        if (argv[i + 1]) {
            putchar(' '); // 参数间用空格分隔
        }
    }

    // 根据 -n 决定是否换行
    if (newline) {
        putchar('\n');
    }

    return 0; // 成功
}

int exec_builtin(t_cmd* cmd, t_shell* sh)
{
    // 根据 cmd->argv 执行内建命令

    if (strcmp(cmd->argv[0], "echo") == 0) {
        // 实现 echo 命令
        return echo_impl(cmd);
    } else if (strcmp(cmd->argv[0], "cd") == 0) {
        // 实现 cd 命令
        if (cmd->argv[1]) {
            if (chdir(cmd->argv[1]) == -1) {
                perror("cd");
                return 1; // 返回错误码
            }
        } else {
            fprintf(stderr, "cd: missing argument\n");
            return 1;
        }
        return 0; // 成功
    } else if (strcmp(cmd->argv[0], "pwd") == 0) {
        // 实现 pwd 命令
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("pwd");
            return 1; // 返回错误码
        }
        return 0; // 成功
    } else if (strcmp(cmd->argv[0], "env") == 0) {
        // 实现 env 命令
        for (char** env = sh->envp; *env != NULL; env++) {
            printf("%s\n", *env);
        }
        return 0; // 成功
    } else if (strcmp(cmd->argv[0], "export") == 0) {
        // 实现 export 命令
        if (cmd->argv[1]) {
            char* var = strdup(cmd->argv[1]); // ✅ 动态分配副本
            char* eq = strchr(var, '=');
            if (eq) {
                *eq = '\0'; // 分离变量名和值
                if (setenv(var, eq + 1, 1) == -1) {
                    perror("export");
                    free(var);
                    return 1; // 返回错误码
                }
                free(var);
                return 0;
            } else {
                fprintf(stderr, "export: invalid format\n");
                free(var);
                return 1;
            }
        } else {
            fprintf(stderr, "export: missing argument\n");
            return 1; // 返回错误码
        }
    } else if (strcmp(cmd->argv[0], "unset") == 0) {
        // 实现 unset 命令
        if (cmd->argv[1]) {
            if (unsetenv(cmd->argv[1]) == -1) {
                perror("unset");
                return 1; // 返回错误码
            }
            return 0; // 成功
        } else {
            fprintf(stderr, "unset: missing argument\n");
            return 1; // 返回错误码
        }
    } else if (strcmp(cmd->argv[0], "exit") == 0) {
        // 实现 exit 命令
        int exit_code = cmd->argv[1] ? atoi(cmd->argv[1]) : 0;
        exit(exit_code);
    } else if (strcmp(cmd->argv[0], "help") == 0) {
        // 实现 help 命令
        builtin_help();
        return 0; // 成功
    }
    return 0; // make LSP happy
}
