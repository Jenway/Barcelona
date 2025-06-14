#if !defined(MINISHELL_H)
#define MINISHELL_H
#include <unistd.h>
#include <stdbool.h>

/* Tokens */
typedef enum e_token_type {
    TOKEN_WORD,         // A sequence of non-special characters, e.g., "ls", "hello-world"
    TOKEN_VARIABLE,     // A variable, e.g., $VAR, $?
    
    TOKEN_PIPE,         // |
    TOKEN_REDIR_IN,     // <
    TOKEN_REDIR_OUT,    // >
    TOKEN_REDIR_APPEND, // >>
    TOKEN_HEREDOC,      // <<

    TOKEN_SQUOTE,       // ' (single quote)
    TOKEN_DQUOTE,       // " (double quote)
    
    TOKEN_SPACE,        // One or more whitespace characters
    TOKEN_EOF           // End of the input string
} t_token_type;

typedef struct s_token {
    t_token_type type;
    char* value; // 原始字符串: 例如 "ls"、">"、"file.txt"、"$HOME"
    struct s_token* next;
} t_token;

/* Segment*/
// A segment is a piece of a final argument string
typedef enum e_segment_type {
    SEG_LITERAL,        // A literal string, e.g., 'ls' or '-l'
    SEG_VARIABLE,       // An unquoted variable, e.g., $HOME
    SEG_SQUOTE_LITERAL, // A literal inside single quotes (can be merged with SEG_LITERAL)
    SEG_DQUOTE_LITERAL, // A literal part inside double quotes, e.g., "hello "
    SEG_DQUOTE_VAR      // A variable part inside double quotes, e.g., "$VAR"
} t_segment_type;

typedef struct s_segment {
    t_segment_type type;
    char* value;             // Raw value (e.g., "HOME" for $HOME)
    struct s_segment* next;
} t_segment;

// An argument is a list of segments that will be joined together
typedef struct s_argument {
    t_segment* segments;
    struct s_argument* next; // The next argument in the command
} t_argument;

/* Redirect */
typedef enum e_redirect_type {
    REDIR_IN, // <
    REDIR_OUT, // >
    REDIR_APPEND, // >>
    REDIR_HEREDOC // <<
} t_redirect_type;

typedef struct s_redirect {
    t_redirect_type type; // 重定向类型
    char* file; // 文件路径或 heredoc limiter
    struct s_redirect* next;
} t_redirect;

/* Command */
typedef struct s_cmd {
    t_argument* args;       // The command and its arguments
    t_redirect* in_redir;
    t_redirect* out_redir;
    char** argv;            // This will be filled in by the expander stage
    char* path;
    int is_builtin;
    struct s_cmd* next;
} t_cmd;

typedef struct s_shell {
    t_cmd* cmds; // 命令链表（从解析器生成）
    char** envp; // 环境变量（用于 execve）
    int last_exit; // 上一个命令退出码（用于 $?）
    int interactive; // 是否是交互式 shell
} t_shell;

/* ===================== Tokenizer ==================== */
t_token* tokenize(const char* input,int *error);

/* ===================== Parser ======================= */
t_cmd* parse(t_token* token);

/* ===================== Expander ===================== */
void expand_commands(t_shell* sh);

/* ===================== Executor ===================== */
int execute_cmd_chain(t_shell* sh);

// builtins
int is_builtin(const char* cmd);
int exec_builtin(t_cmd* cmd, t_shell* sh);

/* ===================== REPL ========================= */
void setup_signals();
int minishell_loop(bool interactive);

/* ===================== Utils ======================== */
t_token* new_token(t_token_type type, const char* value);

void print_tokens(t_token* token);
void free_arguments(t_argument* arg);

void free_cmd_list(t_cmd* cmds);
void free_tokens(t_token* token);

#define DEFINE_LIST_LEN_FUNC(type, member)          \
static inline size_t type##_len(type *head) {       \
    size_t len = 0;                                 \
    for (; head; head = head->member)               \
        len++;                                      \
    return len;                                     \
}

#endif