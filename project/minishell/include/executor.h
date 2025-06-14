#ifndef EXECUTOR_H
# define EXECUTOR_H

#include <minishell.h>

void perror_exit(const char* msg);
int** alloc_pipes(int n_pipes);
void dealloc_pipes(int** pipes, int n_pipes);
void close_unused_pipes(int** pipes, int n_pipes, int i);

int get_in_fd(t_cmd* cmd, int prev_pipe_read);
int get_out_fd(t_cmd* cmd, int next_pipe_write);
void close_all_fds_except(int keep1, int keep2);

void setup_stdio(int in_fd, int out_fd);

#endif
