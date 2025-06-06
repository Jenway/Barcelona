#include "../pipex.h"
#include <libft.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

static char* join_path(const char* dir, const char* cmd)
{
    size_t len = ft_strlen(dir) + ft_strlen(cmd) + 2;
    char* full = malloc(len);
    if (!full)
        return NULL;
    snprintf(full, len, "%s/%s", dir, cmd);
    return full;
}

char* get_cmd_path(char* cmd, char** envp)
{
    if (!cmd)
        return NULL;

    if (ft_strchr(cmd, '/')) // 是绝对或相对路径
    {
        if (access(cmd, X_OK) == 0)
            return ft_strdup(cmd);
        return NULL;
    }

    // 否则从 PATH 中找
    char* path_line = NULL;
    for (int i = 0; envp[i]; i++) {
        if (ft_strncmp(envp[i], "PATH=", 5) == 0) {
            path_line = envp[i] + 5;
            break;
        }
    }
    if (!path_line)
        return NULL;

    char** paths = ft_split(path_line, ':');
    if (!paths)
        return NULL;

    char* full = NULL;
    for (int i = 0; paths[i]; i++) {
        full = join_path(paths[i], cmd);
        if (full && access(full, X_OK) == 0)
            break;
        free(full);
        full = NULL;
    }

    for (int i = 0; paths[i]; i++)
        free(paths[i]);
    free(paths);

    return full;
}
