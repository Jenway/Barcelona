#include <minishell.h>
#include <unistd.h>
#include <unistd.h>

#ifdef RELEASE
#include <stdio.h>
#endif

int main(int argc, char** argv, char** envp)
{
    (void)argc;
    (void)argv;

    // 初始化环境
    __environ = envp;

    // 设置信号处理
    setup_signals();

#ifdef RELEASE
    // 在 RELEASE 模式下，输出一些幽默的消息
    printf("Goodbye!\n");
#endif

    // 判断是否是交互式终端（stdin是否连接到 tty）
    bool interactive = isatty(STDIN_FILENO);

    // 主循环
    return minishell_loop(interactive);
}