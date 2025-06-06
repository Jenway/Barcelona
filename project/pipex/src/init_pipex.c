#include "../pipex.h"
#include <stdlib.h>
#include <string.h>

t_pipex* init_pipex(int argc, char** argv, char** envp)
{
    t_pipex* px = malloc(sizeof(t_pipex));
    if (!px)
        return NULL;

    px->is_here_doc = (strcmp(argv[1], "here_doc") == 0);
    px->infile = px->is_here_doc ? NULL : argv[1];
    px->outfile = argv[argc - 1];
    px->limiter = px->is_here_doc ? argv[2] : NULL;
    px->envp = envp;

    px->cmds = parse_commands(argc, argv, envp, px->is_here_doc);
    if (!px->cmds) {
        free(px);
        return NULL;
    }
    return px;
}
