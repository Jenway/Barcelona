#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <minishell.h>

t_token* new_token(t_token_type type, const char* value)
{
    t_token* token = calloc(1, sizeof(t_token));
    if (!token)
        exit(EXIT_FAILURE);
    token->type = type;
    token->value = strdup(value);
    return token;
}

void free_tokens(t_token* token)
{
    while (token) {
        t_token* next = token->next;
        free(token->value);
        free(token);
        token = next;
    }
}

// Helper to print a single token
static void print_token(t_token* token)
{
    if (!token)
        return;
    const char* type_str;
    switch (token->type) {
    case TOKEN_WORD:
        type_str = "WORD";
        break;
    case TOKEN_VARIABLE:
        type_str = "VAR";
        break;
    case TOKEN_PIPE:
        type_str = "PIPE";
        break;
    case TOKEN_REDIR_IN:
        type_str = "IN";
        break;
    case TOKEN_REDIR_OUT:
        type_str = "OUT";
        break;
    case TOKEN_REDIR_APPEND:
        type_str = "APPEND";
        break;
    case TOKEN_HEREDOC:
        type_str = "HEREDOC";
        break;
    case TOKEN_SQUOTE:
        type_str = "SQUOTE";
        break;
    case TOKEN_DQUOTE:
        type_str = "DQUOTE";
        break;
    case TOKEN_SPACE:
        type_str = "SPACE";
        break;
    case TOKEN_EOF:
        type_str = "EOF";
        break;
    default:
        type_str = "UNKNOWN";
        break;
    }
    // For space, don't print value. For others, print value.
    if (token->type == TOKEN_SPACE) {
        printf("[%s] -> ", type_str);
    } else {
        printf("[%s:\"%s\"] -> ", type_str, token->value);
    }
}

// Helper to print a token list
void print_tokens(t_token* token)
{
    while (token) {
        print_token(token);
        if (token->type == TOKEN_EOF)
            break;
        token = token->next;
    }
    printf("\n");
}
void free_arguments(t_argument* arg)
{
    while (arg) {
        t_argument* next = arg->next;
        t_segment* seg = arg->segments;
        while (seg) {
            t_segment* seg_next = seg->next;
            free(seg->value);
            free(seg);
            seg = seg_next;
        }
        free(arg);
        arg = next;
    }
}
static void free_redirects(t_redirect* redir)
{
    while (redir) {
        t_redirect* next = redir->next;
        free(redir->file);
        free(redir);
        redir = next;
    }
}

void free_cmd_list(t_cmd* cmds)
{
    while (cmds) {
        t_cmd* next = cmds->next;
        if (cmds->argv) {
            for (int i = 0; cmds->argv[i]; ++i)
                free(cmds->argv[i]);
            free(cmds->argv);
        }
        free_redirects(cmds->in_redir);
        free_redirects(cmds->out_redir);
        free(cmds);
        cmds = next;
    }
}
