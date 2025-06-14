#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <minishell.h>

static char* parse_variable_name(const char** input);

typedef enum tokenizer_state {
    STAT_NORMAL, // Normal state, not inside quotes
    STAT_SQUOTE, // Inside single quotes
    STAT_DQUOTE // Inside double quotes
} t_tokenizer_state;

t_token* tokenize(const char* input, int* error)
{
    t_token* head = NULL;
    t_token** current_token_ptr = &head;
    t_tokenizer_state state = STAT_NORMAL;

    while (*input) {
        t_token* token = NULL;
        const char* start = input;

        // 1. Handle Whitespace
        if (isspace(*input)) {
            while (*input && isspace(*input)) {
                input++;
            }
            token = new_token(TOKEN_SPACE, " "); // Value doesn't matter, type is key
        }
        // 2. Handle multi-character operators: >>, <<
        else if (strncmp(input, ">>", 2) == 0) {
            token = new_token(TOKEN_REDIR_APPEND, ">>");
            input += 2;
        } else if (strncmp(input, "<<", 2) == 0) {
            token = new_token(TOKEN_HEREDOC, "<<");
            input += 2;
        }
        // 3. Handle single-character operators: |, <, >
        else if (*input == '|') {
            token = new_token(TOKEN_PIPE, "|");
            input++;
        } else if (*input == '<') {
            token = new_token(TOKEN_REDIR_IN, "<");
            input++;
        } else if (*input == '>') {
            token = new_token(TOKEN_REDIR_OUT, ">");
            input++;
        }
        // 4. Handle Quotes
        else if (*input == '\'') {
            input++; // 跳过开头的 '

            if (state == STAT_DQUOTE) {
                token = new_token(TOKEN_WORD, "'"); // Treat as a literal single quote
            } else {
                // 这是新的单引号处理逻辑
                const char* start_quote = input;
                while (*input && *input != '\'') {
                    input++;
                }
                // 创建一个包含单引号内所有内容的 WORD token
                char* value = strndup(start_quote, input - start_quote);
                token = new_token(TOKEN_WORD, value);
                free(value);

                if (*input == '\'') {
                    input++; // 跳过结尾的 '
                } else {
                    fprintf(stderr, "minishell: Syntax error: unclosed single quote\n");
                    *error = 1;
                    goto fail_cleanup; // Exit on error
                }
            }

        } else if (*input == '"') {
            token = new_token(TOKEN_DQUOTE, "\"");
            if (state == STAT_NORMAL) {
                state = STAT_DQUOTE; // Entering double quote state
            } else {
                state = STAT_NORMAL; // Exiting double quote state
            }
            input++;
        }
        // 5. Handle Variables
        else if (*input == '$') {
            input++; // Skip the '$'
            char* var_name = parse_variable_name(&input);
            if (var_name) {
                token = new_token(TOKEN_VARIABLE, var_name);
                free(var_name); // new_token makes its own copy
            } else {
                // It's just a literal '$'
                token = new_token(TOKEN_WORD, "$");
            }
        }
        // 6. Handle a regular WORD
        else {
            while (*input && !isspace(*input) && !strchr("|<>\"'$", *input)) {
                input++;
            }
            char* word_value = strndup(start, input - start);
            token = new_token(TOKEN_WORD, word_value);
            free(word_value);
        }

        // Link the new token to the list
        *current_token_ptr = token;
        current_token_ptr = &token->next;
    }

    *current_token_ptr = new_token(TOKEN_EOF, "EOF");
    return head;

fail_cleanup:
    while (head) {
        t_token* next = head->next;
        free(head->value);
        free(head);
        head = next;
    }
    return NULL; // Return NULL to indicate failure
}

static char* parse_variable_name(const char** input)
{
    const char* start = *input;

    if (**input == '?') {
        (*input)++;
        return strdup("?");
    }

    if (!isalpha(**input) && **input != '_') {
        return NULL; // Not a valid start for a variable name
    }
    (*input)++; // Consume the first character
    while (isalnum(**input) || **input == '_') {
        (*input)++;
    }
    return strndup(start, *input - start);
}