#include "../pipex.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv, char** envp)
{
    if (argc < 5) {
        dprintf(STDERR_FILENO, "Usage: %s file1 cmd1 cmd2 file2\n", argv[0]);
        return 1;
    }
    t_pipex* px = init_pipex(argc, argv, envp);
    if (!px)
        return 1;

    int code = execute_pipeline(px);

    free_cmd_list(px->cmds);
    free(px);
    return code;
}
