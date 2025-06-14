#include <minishell.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static t_segment* new_segment(t_segment_type type, const char* value);
static void append_segment(t_segment** head, t_segment* new_seg);

static t_redirect* parse_redirect(t_token** token_ptr, int* error);
static void handle_redir(t_cmd* cmd, t_redirect* redir);

static t_argument* parse_arguments(t_token** token_ptr, int* error, t_cmd* cmd);
static t_argument* parse_one_argument(t_token** token_ptr, int* error);

static t_segment* parse_single_quoted_segment(t_token** token_ptr, int* error);
static t_segment* parse_double_quoted_segments(t_token** token_ptr, int* error);

t_cmd* parse(t_token* token)
{
    t_cmd* head = NULL;
    t_cmd** curr = &head;
    int error = 0;
    // Ignore leading spaces
    while (token && token->type == TOKEN_SPACE) {
        token = token->next;
    }

    while (token && token->type != TOKEN_EOF && !error) {
        t_cmd* cmd = calloc(1, sizeof(t_cmd));
        if (!cmd)
            return NULL;

        cmd->args = parse_arguments(&token, &error, cmd);
        if (error) {
            free_cmd_list(head); // Proper cleanup needed
            return NULL;
        }

        *curr = cmd;
        curr = &cmd->next;

        if (token && token->type == TOKEN_PIPE) {
            token = token->next;
            // Ignore spaces after a pipe
            while (token && token->type == TOKEN_SPACE) {
                token = token->next;
            }
        } else if (token && token->type != TOKEN_EOF) {
            fprintf(stderr, "Syntax error: unexpected token after command\n");
            error = 1;
            // free_cmd_list(head);
            return NULL;
        }
    }

    return head;
}

// Parses all arguments and handles redirections for a single command
static t_argument* parse_arguments(t_token** token_ptr, int* error, t_cmd* cmd)
{
    t_argument* args_head = NULL;
    t_argument** args_cursor = &args_head;

    while (*token_ptr && (*token_ptr)->type != TOKEN_PIPE && (*token_ptr)->type != TOKEN_EOF) {
        if ((*token_ptr)->type == TOKEN_SPACE) {
            *token_ptr = (*token_ptr)->next; // Skip spaces between arguments
            continue;
        }

        if ((*token_ptr)->type == TOKEN_REDIR_IN
            || (*token_ptr)->type == TOKEN_REDIR_OUT
            || (*token_ptr)->type == TOKEN_REDIR_APPEND
            || (*token_ptr)->type == TOKEN_HEREDOC) {
            t_redirect* redir = parse_redirect(token_ptr, error);
            handle_redir(cmd, redir);
        } else {
            // It's part of an argument
            t_argument* new_arg = parse_one_argument(token_ptr, error);
            if (*error) { /* free memory */
                return NULL;
            }
            *args_cursor = new_arg;
            args_cursor = &new_arg->next;
        }
    }
    return args_head;
}

static t_argument* parse_one_argument(t_token** token_ptr, int* error)
{
    t_argument* arg = calloc(1, sizeof(t_argument));
    t_segment* current_segments = NULL;

    // An argument is a contiguous sequence of WORDs, VARs, and QUOTEs
    while (*token_ptr
        && (*token_ptr)->type != TOKEN_SPACE
        && (*token_ptr)->type != TOKEN_PIPE
        && (*token_ptr)->type != TOKEN_EOF
        && (*token_ptr)->type != TOKEN_REDIR_IN
        && (*token_ptr)->type != TOKEN_REDIR_OUT
        && (*token_ptr)->type != TOKEN_REDIR_APPEND
        && (*token_ptr)->type != TOKEN_HEREDOC) {

        t_token* t = *token_ptr;
        t_segment* new_segs = NULL;

        if (t->type == TOKEN_WORD) {
            new_segs = new_segment(SEG_LITERAL, t->value);
        } else if (t->type == TOKEN_VARIABLE) {
            new_segs = new_segment(SEG_VARIABLE, t->value);
        } else if (t->type == TOKEN_SQUOTE) {
            new_segs = parse_single_quoted_segment(token_ptr, error);
        } else if (t->type == TOKEN_DQUOTE) {
            new_segs = parse_double_quoted_segments(token_ptr, error);
        } else {
            fprintf(stderr, "Syntax error: unexpected token in argument: %s\n", t->value);
            *error = 1;
            // free arg and segments
            return NULL;
        }

        if (*error) { /* free memory */
            return NULL;
        }

        append_segment(&current_segments, new_segs);

        if (t->type != TOKEN_SQUOTE && t->type != TOKEN_DQUOTE) {
            *token_ptr = (*token_ptr)->next;
        }
    }

    arg->segments = current_segments;
    return arg;
}

static void handle_redir(t_cmd* cmd, t_redirect* redir)
{
    if (redir->type == REDIR_IN || redir->type == REDIR_HEREDOC) {
        redir->next = cmd->in_redir;
        cmd->in_redir = redir;
    } else {
        redir->next = cmd->out_redir;
        cmd->out_redir = redir;
    }
}

static t_redirect* parse_redirect(t_token** token_ptr, int* error)
{
    t_redirect* redir = calloc(1, sizeof(t_redirect));
    if (!redir) {
        *error = 1;
        return NULL;
    }
    // 确定重定向类型
    switch ((*token_ptr)->type) {
    case TOKEN_REDIR_IN:
        redir->type = REDIR_IN;
        break;
    case TOKEN_REDIR_OUT:
        redir->type = REDIR_OUT;
        break;
    case TOKEN_REDIR_APPEND:
        redir->type = REDIR_APPEND;
        break;
    case TOKEN_HEREDOC:
        redir->type = REDIR_HEREDOC;
        break;
    default:
        fprintf(stderr, "Parser bug: parse_redirect called on non-redir token\n");
        *error = 1;
        free(redir);
        return NULL;
    }

    // 1. 消费重定向符号本身 (<, >, >>, <<)
    *token_ptr = (*token_ptr)->next;

    // 2. 循环消费重定向符号后面的所有空格
    while (*token_ptr && (*token_ptr)->type == TOKEN_SPACE) {
        *token_ptr = (*token_ptr)->next;
    }

    // 3. 现在我们期望一个文件名，它（必须）是 TOKEN_WORD
    if (!*token_ptr || (*token_ptr)->type != TOKEN_WORD) {
        fprintf(stderr, "Syntax error: expected filename after redirection\n");
        *error = 1;
        free(redir);
        return NULL;
    }

    // 4. 提取文件名并消费掉文件名的 token
    redir->file = strdup((*token_ptr)->value);
    if (!redir->file) {
        perror("strdup");
        *error = 1;
        free(redir);
        return NULL;
    }
    *token_ptr = (*token_ptr)->next; // 消费文件名 token

    return redir;
}

// Helper functions to create and add to our new structures
static t_segment* new_segment(t_segment_type type, const char* value)
{
    t_segment* seg = calloc(1, sizeof(t_segment));
    seg->type = type;
    seg->value = strdup(value);
    return seg;
}

static void append_segment(t_segment** head, t_segment* new_seg)
{
    if (!new_seg)
        return;
    if (!*head) {
        *head = new_seg;
        return;
    }
    t_segment* curr = *head;
    while (curr->next) {
        curr = curr->next;
    }
    curr->next = new_seg;
}
// --- Quote Parsing Logic ---

static t_segment* parse_single_quoted_segment(t_token** token_ptr, int* error)
{
    *token_ptr = (*token_ptr)->next; // Consume opening SQUOTE

    if (!*token_ptr || (*token_ptr)->type == TOKEN_SQUOTE) {
        // Handles empty string ''
        *token_ptr = (*token_ptr)->next; // Consume closing SQUOTE
        return new_segment(SEG_SQUOTE_LITERAL, "");
    }

    if ((*token_ptr)->type == TOKEN_WORD) {
        t_segment* seg = new_segment(SEG_SQUOTE_LITERAL, (*token_ptr)->value);
        *token_ptr = (*token_ptr)->next; // Consume the word
        if (!*token_ptr || (*token_ptr)->type != TOKEN_SQUOTE) {
            fprintf(stderr, "Syntax error: unclosed single quote\n");
            *error = 1;
            free(seg);
            return NULL;
        }
        *token_ptr = (*token_ptr)->next; // Consume closing SQUOTE
        return seg;
    }

    fprintf(stderr, "Syntax error inside single quotes\n");
    *error = 1;
    return NULL;
}

static t_segment* parse_double_quoted_segments(t_token** token_ptr, int* error)
{
    *token_ptr = (*token_ptr)->next; // Consume opening DQUOTE

    t_segment* head = NULL;

    while (*token_ptr && (*token_ptr)->type != TOKEN_DQUOTE) {
        t_token* t = *token_ptr;
        t_segment* new_seg = NULL;

        if (t->type == TOKEN_WORD || t->type == TOKEN_SPACE) {
            new_seg = new_segment(SEG_DQUOTE_LITERAL, t->value);
        } else if (t->type == TOKEN_VARIABLE) {
            new_seg = new_segment(SEG_DQUOTE_VAR, t->value);
        } else {
            new_seg = new_segment(SEG_DQUOTE_LITERAL, t->value);
        }
        append_segment(&head, new_seg);
        *token_ptr = (*token_ptr)->next;
    }

    if (!*token_ptr) {
        fprintf(stderr, "Syntax error: unclosed double quote\n");
        *error = 1;
        // free list
        return NULL;
    }

    *token_ptr = (*token_ptr)->next; // Consume closing DQUOTE

    if (!head) { // Handles empty string ""
        return new_segment(SEG_DQUOTE_LITERAL, "");
    }

    return head;
}