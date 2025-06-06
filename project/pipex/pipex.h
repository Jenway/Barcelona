#if!defined(PIEX_H)
#define PIEX_H
#include <unistd.h>

typedef struct s_cmd
{
    char            **argv;      // 参数数组，如 ["ls", "-l", NULL]
    char            *path;       // 可执行路径，如 "/bin/ls"
    int             pipe_fd[2];  // 与下一个命令的 pipe
    struct s_cmd    *next;
}   t_cmd;

typedef struct s_pipex
{
    int     is_here_doc;    // 是否是 here_doc 模式
    char    *infile;        // 输入文件名
    char    *outfile;       // 输出文件名
    char    *limiter;       // here_doc 的 limiter（如果启用）
    t_cmd   *cmds;          // 命令链表
    char    **envp;         // 环境变量
}   t_pipex;

t_pipex *init_pipex(int argc, char **argv, char **envp);
t_cmd   *parse_commands(int argc, char **argv, char **envp, int is_here_doc);
char    *get_cmd_path(char *cmd, char **envp);
void    free_cmd_list(t_cmd *cmd_list);
void print_cmd_list(t_cmd* cmd_list);

pid_t   safe_fork(void);
void    safe_pipe(int pipe_fd[2]);
void    redirect_fd(int from, int to);
void    perror_exit(const char *msg);
int     open_infile(const char *filename);
int     open_outfile(const char *filename, int append);

int execute_pipeline(t_pipex *px);


#endif // PIPEX_H