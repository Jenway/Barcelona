#include "../pipex.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static void child_process(t_cmd* cmd, int in_fd, int out_fd, char** envp)
{
    if (dup2(in_fd, STDIN_FILENO) < 0) {
        perror_exit("dup2 in_fd");
    }
    if (dup2(out_fd, STDOUT_FILENO) < 0) {
        perror_exit("dup2 out_fd");
    }
    if (access(cmd->path, X_OK) == -1) {
        dprintf(STDERR_FILENO, "pipex: %s: %s\n", cmd->argv[0], strerror(errno));
        exit(127); // shell 通常命令找不到返回127
    }
    execve(cmd->path, cmd->argv, envp);
    perror_exit("execve");
}

int execute_pipeline(t_pipex* px)
{
    int n_cmds = 0;
    t_cmd* cmd = px->cmds;
    int exit_code = 0;

    // 1. 统计命令数量
    for (t_cmd* tmp = cmd; tmp; tmp = tmp->next)
        n_cmds++;

    // 2. 创建 n-1 个 pipe
    int pipes[n_cmds - 1][2];
    for (int i = 0; i < n_cmds - 1; i++)
        safe_pipe(pipes[i]);

    // 3. 打开 infile & outfile
    int infile_fd;
    if (px->is_here_doc) {
        int pipefd[2];
        pipe(pipefd);

        pid_t pid = fork();
        if (pid == 0) {
            // 子进程负责读取用户输入
            close(pipefd[0]);
            char* line = NULL;
            size_t len = 0;
            while (1) {
                write(STDOUT_FILENO, "heredoc> ", 9);
                ssize_t nread = getline(&line, &len, stdin);
                if (nread == -1)
                    break;
                if (strncmp(line, px->limiter, strlen(px->limiter)) == 0 && line[strlen(px->limiter)] == '\n')
                    break;
                write(pipefd[1], line, nread);
            }
            free(line);
            close(pipefd[1]);
            exit(0);
        }
        close(pipefd[1]);
        waitpid(pid, NULL, 0);
        infile_fd = pipefd[0];
    } else {
        infile_fd = open_infile(px->infile);
    }

    int outfile_fd;
    if (px->is_here_doc)
        outfile_fd = open_outfile(px->outfile, 1); // 1 表示 O_APPEND
    else
        outfile_fd = open_outfile(px->outfile, 0); // 0 表示 O_TRUNC

    pid_t pids[n_cmds];

    // 4. 遍历所有命令
    for (int i = 0; i < n_cmds; i++) {
        int in_fd, out_fd;
        if (i == 0)
            in_fd = infile_fd;
        else
            in_fd = pipes[i - 1][0];

        if (i == n_cmds - 1)
            out_fd = outfile_fd;
        else
            out_fd = pipes[i][1];

        pid_t pid = safe_fork();
        if (pid == 0) {
            // 子进程关闭不需要的 pipe 端口
            // 子进程关闭管道端口
            for (int j = 0; j < n_cmds - 1; j++) {
                if (j != i - 1) // 不是输入管道端
                    close(pipes[j][0]);
                if (j != i) // 不是输出管道端
                    close(pipes[j][1]);
            }
            // 关闭子进程中未用的文件描述符
            if (in_fd != infile_fd)
                close(infile_fd);
            if (out_fd != outfile_fd)
                close(outfile_fd);
            child_process(cmd, in_fd, out_fd, px->envp);
        }
        pids[i] = pid;
        if (i > 0)
            close(pipes[i - 1][0]); // 关闭前一个命令的读端
        if (i < n_cmds - 1)
            close(pipes[i][1]); // 关闭当前命令的写端

        cmd = cmd->next;
    }

    close(infile_fd);
    close(outfile_fd);

    // 5. 等待所有子进程
    int status;
    for (int i = 0; i < n_cmds; i++) {
        waitpid(pids[i], &status, 0);
        if (i == n_cmds - 1) {
            if (WIFEXITED(status)) {
                exit_code = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                exit_code = 128 + WTERMSIG(status); // 128 + 信号编号
            } else {
                exit_code = 1; // 未知错误
            }
        }
    }
    return exit_code;
}
