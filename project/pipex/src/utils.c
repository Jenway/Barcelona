#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t safe_fork(void)
{
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    return pid;
}

void safe_pipe(int pipe_fd[2])
{
    if (pipe(pipe_fd) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
}

void redirect_fd(int from, int to)
{
    if (dup2(from, to) < 0) {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
}

void perror_exit(const char* msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int open_infile(const char* filename)
{
    if (access(filename, R_OK) < 0) {
        dprintf(STDERR_FILENO, "pipex: %s: %s\n", filename, strerror(errno));
        return -1;
    }
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        dprintf(STDERR_FILENO, "pipex: %s: %s\n", filename, strerror(errno));
        return -1;
    }
    return fd;
}

int open_outfile(const char* filename, int append)
{
    int flags = O_WRONLY | O_CREAT;
    if (append)
        flags |= O_APPEND;
    else
        flags |= O_TRUNC;
    int fd = open(filename, flags, 0644);
    if (fd < 0) {
        dprintf(STDERR_FILENO, "pipex: %s: %s\n", filename, strerror(errno));
        return -1;
    }
    return fd;
}
