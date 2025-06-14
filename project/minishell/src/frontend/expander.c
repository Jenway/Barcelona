#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <minishell.h>

static char* get_env_value(const char* key, char** envp)
{
    if (!key || !envp)
        return "";
    size_t key_len = strlen(key);
    for (int i = 0; envp[i]; ++i) {
        if (strncmp(envp[i], key, key_len) == 0 && envp[i][key_len] == '=') {
            return envp[i] + key_len + 1;
        }
    }
    return ""; // Return empty string if not found, as per shell behavior
}

// A helper to safely append a string to a dynamically allocated string
static void append_to_string(char** base_str, const char* to_add)
{
    if (!to_add)
        return;
    size_t base_len = (*base_str) ? strlen(*base_str) : 0;
    size_t add_len = strlen(to_add);

    char* new_str = realloc(*base_str, base_len + add_len + 1);
    if (!new_str) {
        perror("realloc in expander");
        exit(EXIT_FAILURE); // Or handle error more gracefully
    }
    memcpy(new_str + base_len, to_add, add_len + 1); // Use memcpy for safety
    *base_str = new_str;
}

// --- Core Expander Logic ---

/**
 * @brief Expands a single argument (which is a list of segments) into a single string.
 * This is the heart of the expander. It processes each segment according to its type.
 * @param arg The argument to expand.
 * @param sh The shell context for environment variables and exit status.
 * @return A newly allocated string representing the final expanded argument.
 */
static char* expand_argument(t_argument* arg, t_shell* sh)
{
    char* final_arg_str = calloc(1, sizeof(char)); // Start with an empty string
    if (!final_arg_str)
        exit(EXIT_FAILURE);

    for (t_segment* seg = arg->segments; seg; seg = seg->next) {
        switch (seg->type) {
        case SEG_LITERAL:
        case SEG_SQUOTE_LITERAL:
        case SEG_DQUOTE_LITERAL:
            // For all literal types, just append their value.
            append_to_string(&final_arg_str, seg->value);
            break;

        case SEG_VARIABLE:
        case SEG_DQUOTE_VAR:
            // For variable types, get the value from the environment and append it.
            if (strcmp(seg->value, "?") == 0) {
                char exit_str[12];
                snprintf(exit_str, sizeof(exit_str), "%d", sh->last_exit);
                append_to_string(&final_arg_str, exit_str);
            } else {
                const char* env_val = get_env_value(seg->value, sh->envp);
                append_to_string(&final_arg_str, env_val);
            }
            break;
        }
    }
    return final_arg_str;
}

/**
 * @brief Main expander function. Iterates through all commands and their arguments,
 * expanding them and filling the `argv` array for each command.
 * @param sh The main shell struct containing commands, envp, etc.
 */
void expand_commands(t_shell* sh)
{
    for (t_cmd* cmd = sh->cmds; cmd; cmd = cmd->next) {
        if (!cmd->args) {
            // Command might have only redirections, e.g., `> file`
            cmd->argv = calloc(1, sizeof(char*)); // Still need a NULL-terminated argv
            cmd->argv[0] = NULL;
            continue;
        }

        // 1. Count the number of arguments to allocate argv
        size_t argc = 0;
        for (t_argument* arg = cmd->args; arg; arg = arg->next) {
            argc++;
        }
        cmd->argv = malloc(sizeof(char*) * (argc + 1));
        if (!cmd->argv)
            exit(EXIT_FAILURE);

        // 2. Expand each argument and fill argv
        size_t i = 0;
        for (t_argument* arg = cmd->args; arg; arg = arg->next) {
            cmd->argv[i++] = expand_argument(arg, sh);
        }
        cmd->argv[i] = NULL; // NULL-terminate the argv array

        // 3. (Optional but good practice) Free the parsed AST part for arguments
        free_arguments(cmd->args);
        cmd->args = NULL;
    }
}
