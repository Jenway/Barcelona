#include <minishell.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv, char** envp)
{
    (void)argc;
    (void)argv;

    // 初始化环境
    __environ = envp;

    // 设置信号处理
    setup_signals();

    // 启动主循环
    minishell_loop();

    printf("Goodbye!\n");
    return 0;
}