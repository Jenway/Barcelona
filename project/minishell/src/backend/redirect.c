#include <executor.h>
#include <fcntl.h>
#include <minishell.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

void perror_exit(const char* msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

// 计算管道数量并分配管道数组
int** alloc_pipes(int n_pipes)
{
    int** pipes = malloc(n_pipes * sizeof(int*));
    if (!pipes)
        perror_exit("malloc pipes");
    for (int i = 0; i < n_pipes; i++) {
        pipes[i] = malloc(2 * sizeof(int));
        if (!pipes[i])
            perror_exit("malloc pipe");
        if (pipe(pipes[i]) == -1) {
            perror_exit("pipe");
        }
    }
    return pipes;
}

void dealloc_pipes(int** pipes, int n_pipes)
{
    for (int i = 0; i < n_pipes; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
        free(pipes[i]);
    }
    free(pipes);
}

int get_in_fd(t_cmd* cmd, int prev_pipe_read)
{
    if (cmd->in_redir) {
        int fd = open(cmd->in_redir->file, O_RDONLY);
        if (fd == -1)
            perror_exit("open input redir");
        return fd;
    } else {
        return prev_pipe_read;
    }
}

int get_out_fd(t_cmd* cmd, int next_pipe_write)
{
    if (cmd->out_redir) {
        int flags = O_WRONLY | O_CREAT;
        if (cmd->out_redir->type == REDIR_OUT)
            flags |= O_TRUNC;
        else if (cmd->out_redir->type == REDIR_APPEND)
            flags |= O_APPEND;
        else
            perror_exit("unsupported redirection type");
        int fd = open(cmd->out_redir->file, flags, 0644);
        if (fd == -1)
            perror_exit("open output redir");
        return fd;
    } else {
        return next_pipe_write;
    }
}

void close_unused_pipes(int** pipes, int n_pipes, int i)
{
    for (int j = 0; j < n_pipes; j++) {
        if (j != i - 1 && j != i) {
            close(pipes[j][0]);
            close(pipes[j][1]);
        }
    }
}

void close_all_fds_except(int keep1, int keep2)
{
    int max_fd = sysconf(_SC_OPEN_MAX);
    for (int fd = 3; fd < max_fd; fd++) {
        if (fd != keep1 && fd != keep2) {
            close(fd);
        }
    }
}

void setup_stdio(int in_fd, int out_fd)
{
    if (in_fd != STDIN_FILENO) {
        dup2(in_fd, STDIN_FILENO);
        close(in_fd);
    }
    if (out_fd != STDOUT_FILENO) {
        dup2(out_fd, STDOUT_FILENO);
        close(out_fd);
    }
}
