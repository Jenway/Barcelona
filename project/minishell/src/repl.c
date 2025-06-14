#include <minishell.h>
#include <stdio.h>
// 不要更改顺序
#include <readline/history.h>
#include <readline/readline.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// SIGINT 处理函数 (Ctrl+C)
static void handle_sigint(int sig)
{
    (void)sig;
    printf("\n"); // 新行
    rl_on_new_line(); // 告诉 readline 我们在新行上
    rl_replace_line("", 0); // 清除当前行
    rl_redisplay(); // 重新显示提示符
}
// 设置信号处理
void setup_signals()
{
    signal(SIGINT, handle_sigint);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
}

// 自定义提示符
char* custom_prompt()
{
    static char prompt[256];
    char cwd[256];
    char* user = getenv("USER");
    char* hostname = getenv("HOSTNAME");

    if (!user)
        user = "user";
    if (!hostname)
        hostname = "localhost";

    if (getcwd(cwd, sizeof(cwd))) {
        // 如果在家目录，显示 ~ 代替完整路径
        char* home = getenv("HOME");
        if (home && strstr(cwd, home) == cwd) {
            snprintf(prompt, sizeof(prompt),
                "\001\033[1;32m\002%s@%s:\001\033[1;34m\002~%s\001\033[1;32m\002$ \001\033[0m\002",
                user, hostname, cwd + strlen(home));
        } else {
            snprintf(prompt, sizeof(prompt),
                "\001\033[1;32m\002%s@%s:\001\033[1;34m\002%.90s\001\033[1;32m\002$ \001\033[0m\002",
                user, hostname, cwd);
        }
    } else {
        snprintf(prompt, sizeof(prompt), "\001\033[1;32m\002minishell$ \001\033[0m\002");
    }

    return prompt;
}

// 主 REPL 循环
int minishell_loop(bool interactive)
{
    char* input = NULL;
    int status = 0;

    char history_file[256] = { 0 };
    char* home = getenv("HOME");

    if (interactive) {
        // 启用历史记录
        using_history();
        stifle_history(100); // 最多保存100条历史

        if (home) {
            snprintf(history_file, sizeof(history_file), "%s/.minishell_history", home);
            read_history(history_file);
        }
    }

    while (1) {
        if (interactive) {

#ifdef RELEASE
            char* prompt = custom_prompt();
#else
            char* prompt = "$ ";
#endif
            input = readline(prompt);
        } else {
            // 非交互模式下从 stdin 读取一整行
            char buffer[4096];
            if (!fgets(buffer, sizeof(buffer), stdin)) {
                break; // Ctrl+D 或输入结束
            } // 去除换行符
            buffer[strcspn(buffer, "\n")] = '\0';
            input = strdup(buffer);
        }

        if (!input) {
            printf("exit\n");
            break; // Ctrl+D 退出
        }

        // 跳过空行
        if (strlen(input) == 0) {
            free(input);
            continue;
        }
        if (strcmp(input, "exit") == 0) {
            free(input);
            break; // 输入 exit 退出
        }

        // 添加历史记录
        if (interactive)
            add_history(input);

        int error = 0;

        // 分词
        t_token* tokens = tokenize(input, &error);
        if (!tokens || error) {
            fprintf(stderr, "Syntax error in input: %s\n", input);
            free(input);
            continue; // 跳过错误输入
        }
        // 解析
        t_cmd* cmd_list = parse(tokens);
        if (!cmd_list) {
            fprintf(stderr, "Parsing error in input: %s\n", input);
            free_tokens(tokens);
            free(input);
            continue; // 跳过错误输入
        }

        t_shell sh = {
            .cmds = cmd_list,
            .envp = __environ,
            .last_exit = status
        };
        // Expander
        expand_commands(&sh); // 准备命令路径等信息
        // Executor
        status = execute_cmd_chain(&sh);
        // 清理命令结构
        free_cmd_list(cmd_list);

#ifdef DEBUG
        printf("Exit status: %d\n", status);
#endif
        // 清理 tokens
        free_tokens(tokens);
        free(input);
    }

    // 保存历史到文件
    if (interactive && home) {
        write_history(history_file);
    }
    return status;
}
