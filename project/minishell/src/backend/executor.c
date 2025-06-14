#include <executor.h>
#include <minishell.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

DEFINE_LIST_LEN_FUNC(t_cmd, next)

static char* search_path(const char* cmd, char** envp);
static char* getenv_from_envp(const char* key, char** envp);
static int is_single_command(t_shell* sh);
static int wait_and_get_status(pid_t pid, int is_last);
static void exec_cmd_in_child(t_cmd* cmd, t_shell* sh, int in_fd, int out_fd, int** pipes, int n_pipes, int i);
static void execve_s(t_cmd* cmd, t_shell* sh);

static void command_error(const char* command, const char* message)
{
    fprintf(stderr, "minishell: %s: %s\n", message, command);
}

int execute_cmd_chain(t_shell* sh)
{
    t_cmd* cmd = sh->cmds;
    int n_cmds = t_cmd_len(cmd);
    int** pipes = alloc_pipes(n_cmds - 1);

    int pids[n_cmds];
    int exit_code = 0;

    for (int i = 0; i < n_cmds; i++, cmd = cmd->next) {
        int in_fd = get_in_fd(cmd, i > 0 ? pipes[i - 1][0] : STDIN_FILENO);
        int out_fd = get_out_fd(cmd, i < n_cmds - 1 ? pipes[i][1] : STDOUT_FILENO);
#ifdef DEBUG
        fprintf(stderr, "[DEBUG] Executing command %d: '%s'\n", i + 1, cmd->argv && cmd->argv[0] ? cmd->argv[0] : "(no argv)");
        fprintf(stderr, "[DEBUG] in_fd=%d, out_fd=%d\n", in_fd, out_fd);
#endif
        cmd->is_builtin = is_builtin(cmd->argv ? cmd->argv[0] : NULL);
        if (cmd->is_builtin && is_single_command(sh)) {
            // builtin 且不是管道链，只执行一次，不 fork，不 wait
            exit_code = exec_builtin(cmd, sh);
            goto cleanup;
        }

        pid_t pid = fork();
        if (pid == 0) {
            exec_cmd_in_child(cmd, sh, in_fd, out_fd, pipes, n_cmds - 1, i);
        }

        pids[i] = pid;
    }
    // After forking ALL children, the parent must close ALL its pipe file descriptors.
    for (int i = 0; i < n_cmds - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    // 等待所有子进程
    for (int i = 0; i < n_cmds; i++)
        exit_code = wait_and_get_status(pids[i], i == n_cmds - 1) == -1 ? -1 : exit_code;

cleanup:
    dealloc_pipes(pipes, n_cmds - 1);
    return exit_code;
}

static void exec_cmd_in_child(t_cmd* cmd, t_shell* sh, int in_fd, int out_fd, int** pipes, int n_pipes, int i)
{
#ifdef DEBUG
    fprintf(stderr, "[DEBUG] cmd='%s', in_redir=%s, out_redir=%s\n",
        cmd->argv && cmd->argv[0] ? cmd->argv[0] : "(no argv)",
        cmd->in_redir ? cmd->in_redir->file : "(null)",
        cmd->out_redir ? cmd->out_redir->file : "(null)");
#endif
    close_unused_pipes(pipes, n_pipes, i);
    close_all_fds_except(in_fd, out_fd);
    setup_stdio(in_fd, out_fd);

    if (cmd->is_builtin) {
        exit(exec_builtin(cmd, sh));
    }
    // Ensure we have a command to execute
    if (!cmd->argv || !cmd->argv[0]) {
        exit(0); // No command, exit gracefully
    }

    if (cmd->argv[0][0] == '/' || cmd->argv[0][0] == '.') {
        cmd->path = strdup(cmd->argv[0]);
    } else {
        cmd->path = search_path(cmd->argv[0], sh->envp);
    }

    execve_s(cmd, sh);
}

// 等待子进程并获取退出状态
static int wait_and_get_status(pid_t pid, int is_last)
{
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return -1; // 错误处理
    }
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        if (is_last) {
            // 如果是最后一个命令，更新 shell 的 last_exit
            // extern t_shell* g_shell; // 假设有全局 shell 变量
            // g_shell->last_exit = exit_code;
        }
        return exit_code;
    } else {
        command_error("", "Command did not exit normally");
        return -1; // 非正常退出
    }
}

static char* getenv_from_envp(const char* key, char** envp)
{
    size_t key_len = strlen(key);
    for (int i = 0; envp[i]; i++) {
        if (strncmp(envp[i], key, key_len) == 0 && envp[i][key_len] == '=') {
            return envp[i] + key_len + 1;
        }
    }
    return NULL;
}
static char* search_path(const char* cmd, char** envp)
{
    if (!cmd || strchr(cmd, '/')) {
        // 如果包含 '/'，视为绝对路径或相对路径，调用者处理
        return NULL;
    }

    char* path_env = getenv_from_envp("PATH", envp);
    if (!path_env)
        path_env = "/bin:/usr/bin"; // fallback

    char* paths = strdup(path_env); // strtok 会修改字符串
    if (!paths)
        return NULL;

    char* saveptr = NULL;
    char* dir = strtok_r(paths, ":", &saveptr);
    while (dir) {
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, cmd);
        if (access(full_path, X_OK) == 0) {
            free(paths);
            return strdup(full_path); // 找到可执行命令，返回路径副本
        }
        dir = strtok_r(NULL, ":", &saveptr);
    }

    free(paths);
    return NULL; // 未找到
}

static int is_single_command(t_shell* sh)
{
    // 如果只有一个命令且没有管道或重定向，则视为单个命令
    return (sh->cmds && !sh->cmds->next && !sh->cmds->in_redir && !sh->cmds->out_redir);
}

static void execve_s(t_cmd* cmd, t_shell* sh)
{
    if (cmd->path == NULL) {
        command_error(cmd->argv[0], "command not found");
        exit(127); // Standard exit code for "command not found"
    }

    execve(cmd->path, cmd->argv, sh->envp);

    /* ============== execve 错误处理 ============== */
    struct stat path_stat;
    stat(cmd->path, &path_stat);
    if (S_ISDIR(path_stat.st_mode)) {
        command_error(cmd->argv[0], "is a directory");
        exit(126);
    }

    switch (errno) {
    case EACCES:
        command_error(cmd->argv[0], "Permission denied");
        exit(126);
        break;
    case ENOENT:
        command_error(cmd->argv[0], "No such file or directory");
        exit(127);
        break;
    default:
        command_error(cmd->argv[0], strerror(errno));
        exit(EXIT_FAILURE);
        break;
    }
    /* ========== End of execve 错误处理 ========== */
}