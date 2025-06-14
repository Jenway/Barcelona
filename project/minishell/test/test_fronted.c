#include "minishell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ===================================================================
//                        Helper Print Functions
// ===================================================================

// Helper to print the final AST in a readable format
void print_ast(t_cmd* cmd_list)
{
    int cmd_idx = 0;
    for (t_cmd* cmd = cmd_list; cmd; cmd = cmd->next) {
        printf("--- Command %d ---\n", ++cmd_idx);
        int arg_idx = 0;
        for (t_argument* arg = cmd->args; arg; arg = arg->next) {
            printf("  Arg %d: ", ++arg_idx);
            for (t_segment* seg = arg->segments; seg; seg = seg->next) {
                const char* seg_type_str;
                switch (seg->type) {
                case SEG_LITERAL:
                    seg_type_str = "LIT";
                    break;
                case SEG_VARIABLE:
                    seg_type_str = "VAR";
                    break;
                case SEG_SQUOTE_LITERAL:
                    seg_type_str = "S_LIT";
                    break;
                case SEG_DQUOTE_LITERAL:
                    seg_type_str = "D_LIT";
                    break;
                case SEG_DQUOTE_VAR:
                    seg_type_str = "D_VAR";
                    break;
                default:
                    seg_type_str = "UNK";
                    break;
                }
                printf("[%s:\"%s\"] ", seg_type_str, seg->value);
            }
            printf("\n");
        }
        // You can add printing for redirections here if needed
        if (cmd->argv) {
            printf("  Expanded argv: ");
            for (int i = 0; cmd->argv[i]; i++) {
                printf("[\"%s\"] ", cmd->argv[i]);
            }
            printf("[NULL]\n");
        }
        printf("-------------------\n");
    }
}

// ===================================================================
//                         Test Cases
// ===================================================================

// A structure to hold test cases
typedef struct {
    const char* description;
    const char* input;
} test_case;

void run_full_test(const char* description, const char* input, t_shell* sh)
{
    printf("\n========================================================\n");
    printf("Testing: %s\n", description);
    printf("Input: '%s'\n", input);
    printf("--------------------------------------------------------\n");

    // 1. --- Test Tokenizer ---
    printf("1. Tokenizer Output:\n");
    t_token* tokens = tokenize(input);
    print_tokens(tokens);
    // Note: Assuming tokenizer now handles single quotes as one WORD
    // and is generally more robust as per our last discussion.

    // 2. --- Test Parser ---
    printf("\n2. Parser Output (AST):\n");
    t_cmd* ast = parse(tokens);
    print_ast(ast);

    // 3. --- Test Expander ---
    if (ast) {
        printf("\n3. Expander Output (Final argv):\n");
        sh->cmds = ast; // Link AST to shell for expander
        expand_commands(sh);
        print_ast(ast); // Print again to show the expanded argv
    } else {
        printf("\n3. Expander Skipped (Parser failed)\n");
    }

    // Cleanup
    // free_tokens(tokens);
    // free_cmd_list(ast); // You need a robust cleanup function
    sh->cmds = NULL; // Unlink for next test
    printf("========================================================\n");
}

int main(void)
{
    // Setup a mock shell environment for the expander
    t_shell sh;
    sh.last_exit = 42; // Set a known exit status for $?
    char* envp[] = {
        "USER=testuser",
        "HOME=/home/testuser",
        "VAR=some_value",
        NULL
    };
    sh.envp = envp;
    sh.cmds = NULL;

    // --- Define Test Cases ---
    test_case tests[] = {
        { "Simple command", "ls -l" },
        { "Pipe", "ls -l | wc -l" },
        { "Single quotes", "echo 'hello world'" },
        { "Double quotes with literals", "echo \"hello world\"" },
        { "Double quotes with variable", "echo \"user is $USER\"" },
        { "Mixed quotes", "echo 'user is $USER' \"user is $USER\"" },
        { "Empty quotes", "echo \"\" ''" },
        { "Variable expansion", "echo $USER $VAR $FAKEVAR" },
        { "Exit code expansion", "echo status was $?" },
        { "Complex case with quotes and vars", "grep \"$VAR\" 'file.txt'" },
        { "Adjacent quotes concatenation", "echo \"hello\"' world'" },
        { "Redirection", "cat < file.txt > out.log" },
        { "Subtle spacing and quotes", "echo '  a b  '\" c d \"" },
        { "Unclosed quote test (optional, for error handling)", "echo \"hello" },
        { "Mixture of SQuote and DQuote", "echo \"aspas ->\'\"" },
        { "Mixture of SQuote and DQuote", "echo \"aspas -> \' \"" },
        { NULL, NULL } // Sentinel to mark the end
    };

    // --- Run All Tests ---
    for (int i = 0; tests[i].input != NULL; i++) {
        run_full_test(tests[i].description, tests[i].input, &sh);
    }

    return 0;
}