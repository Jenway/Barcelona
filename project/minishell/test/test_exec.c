#include <minishell.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// 辅助函数：创建临时文件
void create_temp_file(const char* filename, const char* content)
{
    FILE* f = fopen(filename, "w");
    if (f) {
        fprintf(f, "%s", content);
        fclose(f);
    }
}

// 辅助函数：检查文件内容
int file_contains(const char* filename, const char* content)
{
    FILE* f = fopen(filename, "r");
    if (!f)
        return 0;

    char buffer[256];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[len] = '\0';
    fclose(f);

    return strstr(buffer, content) != NULL;
}

// 辅助函数：检查文件是否存在
int file_exists(const char* filename)
{
    return access(filename, F_OK) == 0;
}

// 伪造的 t_shell 和 t_cmd 结构体用于测试
t_redirect* make_redir(const char* file, t_redirect_type type)
{
    t_redirect* redir = calloc(1, sizeof(t_redirect));
    redir->file = strdup(file);
    redir->type = type;
    return redir;
}

t_cmd* make_cmd(char** argv, const char* path, int is_builtin)
{
    t_cmd* cmd = calloc(1, sizeof(t_cmd));
    cmd->argv = argv;
    cmd->path = path ? strdup(path) : NULL;
    cmd->is_builtin = is_builtin;
    return cmd;
}

t_shell* make_shell(t_cmd* cmds)
{
    t_shell* sh = calloc(1, sizeof(t_shell));
    sh->cmds = cmds;
    sh->envp = __environ;
    return sh;
}

// 保存和恢复环境变量
void save_environment(char** env_backup)
{
    extern char** environ;
    for (int i = 0; environ[i]; i++) {
        env_backup[i] = strdup(environ[i]);
    }
}

void restore_environment(char** env_backup)
{
    extern char** environ;
    for (int i = 0; env_backup[i]; i++) {
        setenv(strtok(env_backup[i], "="), strtok(NULL, ""), 1);
    }
}

// 测试用例
void test_single_builtin_cd()
{
    printf("[TEST] single builtin: cd\n");
    char* argv[] = { "cd", "/", NULL };
    t_cmd* cmd = make_cmd(argv, NULL, 1);
    t_shell* sh = make_shell(cmd);

    char original_cwd[1024];
    getcwd(original_cwd, sizeof(original_cwd));

    int r = execute_cmd_chain(sh);
    assert(r == 0);

    char cwd[1024];
    assert(getcwd(cwd, sizeof(cwd)) && strcmp(cwd, "/") == 0);

    chdir(original_cwd); // 恢复原始目录
    printf("\t[PASS]\n");
}

void test_external_ls()
{
    printf("[TEST] external command: /bin/ls\n");
    char* argv[] = { "ls", "-l", ".", NULL };
    t_cmd* cmd = make_cmd(argv, "/bin/ls", 0);
    t_shell* sh = make_shell(cmd);

    int r = execute_cmd_chain(sh);
    assert(r == 0);
    printf("\t[PASS]\n");
}

void test_pipe_ls_wc()
{
    printf("[TEST] pipeline: /bin/ls | /usr/bin/wc -l\n");
    char* argv1[] = { "ls", NULL };
    char* argv2[] = { "wc", "-l", NULL };
    t_cmd* cmd1 = make_cmd(argv1, "/bin/ls", 0);
    t_cmd* cmd2 = make_cmd(argv2, "/usr/bin/wc", 0);
    cmd1->next = cmd2;
    t_shell* sh = make_shell(cmd1);

    int r = execute_cmd_chain(sh);
    assert(r == 0);
    printf("\t[PASS]\n");
}

void test_long_pipe_chain()
{
    printf("[TEST] long pipeline: /bin/echo -e 'apple\\nbanana\\napricot\\norange' | /bin/grep ap | /usr/bin/wc -l\n");

    // echo 输出多行文本
    char* argv1[] = { "echo", "-e", "apple\nbanana\napricot\norange", NULL };
    // grep 过滤包含 "ap" 的行
    char* argv2[] = { "grep", "ap", NULL };
    // wc -l 统计匹配行数
    char* argv3[] = { "wc", "-l", NULL };

    t_cmd* cmd1 = make_cmd(argv1, "/bin/echo", 0);
    t_cmd* cmd2 = make_cmd(argv2, "/bin/grep", 0);
    t_cmd* cmd3 = make_cmd(argv3, "/usr/bin/wc", 0);

    cmd1->next = cmd2;
    cmd2->next = cmd3;

    t_shell* sh = make_shell(cmd1);

    int r = execute_cmd_chain(sh);
    assert(r == 0);

    printf("\t[PASS]\n");
}

void test_output_redirection()
{
    printf("[TEST] output redirection: echo 'test' > test_output.txt\n");

    // 确保文件不存在
    unlink("test_output.txt");

    char* argv[] = { "echo", "test", NULL };
    t_cmd* cmd = make_cmd(argv, "/bin/echo", 0);

    // 添加输出重定向
    t_redirect* redir = make_redir("test_output.txt", REDIR_OUT);
    cmd->out_redir = redir;

    t_shell* sh = make_shell(cmd);

    int r = execute_cmd_chain(sh);
    assert(r == 0);

    // 验证文件内容
    assert(file_exists("test_output.txt"));
    assert(file_contains("test_output.txt", "test"));

    unlink("test_output.txt"); // 清理
    printf("\t[PASS]\n");
}

void test_append_redirection()
{
    printf("[TEST] append redirection: echo 'line1' >> test_append.txt | echo 'line2' >> test_append.txt\n");

    // 创建初始文件
    create_temp_file("test_append.txt", "existing content\n");

    // 第一次追加
    char* argv1[] = { "echo", "line1", NULL };
    t_cmd* cmd1 = make_cmd(argv1, "/bin/echo", 0);
    t_redirect* redir1 = make_redir("test_append.txt", REDIR_APPEND);
    cmd1->out_redir = redir1;

    // 第二次追加
    char* argv2[] = { "echo", "line2", NULL };
    t_cmd* cmd2 = make_cmd(argv2, "/bin/echo", 0);
    t_redirect* redir2 = make_redir("test_append.txt", REDIR_APPEND);
    cmd2->out_redir = redir2;

    cmd1->next = cmd2;
    t_shell* sh = make_shell(cmd1);

    int r = execute_cmd_chain(sh);
    assert(r == 0);

    // 验证文件内容
    assert(file_exists("test_append.txt"));
    assert(file_contains("test_append.txt", "existing content"));
    assert(file_contains("test_append.txt", "line1"));
    assert(file_contains("test_append.txt", "line2"));

    unlink("test_append.txt"); // 清理
    printf("\t[PASS]\n");
}

void test_input_redirection()
{
    printf("[TEST] input redirection: wc -l < test_input.txt\n");

    // 创建测试文件
    create_temp_file("test_input.txt", "line1\nline2\nline3\n");

    char* argv[] = { "wc", "-l", NULL };
    t_cmd* cmd = make_cmd(argv, "/usr/bin/wc", 0);

    // 添加输入重定向
    t_redirect* redir = make_redir("test_input.txt", REDIR_IN);
    cmd->in_redir = redir;

    t_shell* sh = make_shell(cmd);

    int r = execute_cmd_chain(sh);
    assert(r == 0);

    unlink("test_input.txt"); // 清理
    printf("\t[PASS]\n");
}

void test_builtin_env()
{
    printf("[TEST] builtin command: env\n");

    // 设置测试环境变量
    setenv("TEST_ENV_VAR", "test_value", 1);

    char* argv[] = { "env", NULL };
    t_cmd* cmd = make_cmd(argv, NULL, 1);
    char* argv1[] = { "grep", "TEST_ENV_VAR", NULL };
    t_cmd* cmd1 = make_cmd(argv1, "/bin/grep", 0);
    cmd->next = cmd1;
    t_shell* sh = make_shell(cmd);

    int r = execute_cmd_chain(sh);
    assert(r == 0);

    unsetenv("TEST_ENV_VAR"); // 清理
    printf("\t[PASS]\n");
}

void test_builtin_export()
{
    printf("[TEST] builtin command: export\n");

    // 确保变量不存在
    unsetenv("TEST_EXPORT_VAR");

    char* argv[] = { "export", "TEST_EXPORT_VAR=test_value", NULL };
    t_cmd* cmd = make_cmd(argv, NULL, 1);
    t_shell* sh = make_shell(cmd);

    int r = execute_cmd_chain(sh);
    assert(r == 0);

    // 验证变量已设置
    const char* value = getenv("TEST_EXPORT_VAR");
    assert(value != NULL);
    assert(strcmp(value, "test_value") == 0);

    unsetenv("TEST_EXPORT_VAR"); // 清理
    printf("\t[PASS]\n");
}

void test_builtin_unset()
{
    printf("[TEST] builtin command: unset\n");

    // 设置变量
    setenv("TEST_UNSET_VAR", "test_value", 1);

    char* argv[] = { "unset", "TEST_UNSET_VAR", NULL };
    t_cmd* cmd = make_cmd(argv, NULL, 1);
    t_shell* sh = make_shell(cmd);

    int r = execute_cmd_chain(sh);
    assert(r == 0);

    // 验证变量已删除
    assert(getenv("TEST_UNSET_VAR") == NULL);
    printf("\t[PASS]\n");
}

void test_command_not_found()
{
    printf("[TEST] command not found: non_existent_command\n");

    char* argv[] = { "non_existent_command", NULL };
    t_cmd* cmd = make_cmd(argv, "/path/does/not/exist", 0);
    t_shell* sh = make_shell(cmd);

    int r = execute_cmd_chain(sh);
    assert(r != 0); // 应该返回错误
    printf("\t[PASS]\n");
}

void test_redirection_error()
{
    printf("[TEST] redirection error: cat < non_existent_file.txt\n");

    // 确保文件不存在
    unlink("non_existent_file.txt");

    char* argv[] = { "cat", NULL };
    t_cmd* cmd = make_cmd(argv, "/bin/cat", 0);

    // 添加输入重定向到不存在的文件
    t_redirect* redir = make_redir("non_existent_file.txt", REDIR_IN);
    cmd->in_redir = redir;

    t_shell* sh = make_shell(cmd);

    int r = execute_cmd_chain(sh);
    assert(r != 0); // 应该返回错误
    printf("\t[PASS]\n");
}

void test_pipe_with_redirection()
{
    printf("[TEST] pipe with redirection: ls -l | grep 'minishell' > output.txt\n");

    // 确保文件不存在
    unlink("output.txt");

    // 第一个命令: ls -l
    char* argv1[] = { "ls", "-l", NULL };
    t_cmd* cmd1 = make_cmd(argv1, "/bin/ls", 0);

    // 第二个命令: grep 'minishell' 并重定向输出
    char* argv2[] = { "grep", "minishell", NULL };
    t_cmd* cmd2 = make_cmd(argv2, "/bin/grep", 0);

    // 添加输出重定向
    t_redirect* redir = make_redir("output.txt", REDIR_OUT);
    cmd2->out_redir = redir;

    cmd1->next = cmd2;
    t_shell* sh = make_shell(cmd1);

    int r = execute_cmd_chain(sh);
    assert(r == 0);

    // 验证输出文件已创建
    assert(file_exists("output.txt"));
    unlink("output.txt"); // 清理
    printf("\t[PASS]\n");
}

int main()
{
    char* original_cwd = getcwd(NULL, 0);

    // 环境变量备份
    char* env_backup[100] = { 0 };
    extern char** environ;
    for (int i = 0; environ[i]; i++) {
        env_backup[i] = strdup(environ[i]);
    }

    // 运行测试
    test_single_builtin_cd();
    chdir(original_cwd); // 回到原目录

    test_external_ls();
    test_pipe_ls_wc();
    test_long_pipe_chain();

    test_output_redirection();
    test_append_redirection();
    test_input_redirection();

    test_builtin_env();
    test_builtin_export();
    test_builtin_unset();

    test_command_not_found();
    test_redirection_error();

    test_pipe_with_redirection();

    // 清理
    free(original_cwd);

    // 恢复环境变量
    for (int i = 0; env_backup[i]; i++) {
        char* var = strtok(env_backup[i], "=");
        char* value = strtok(NULL, "");
        setenv(var, value ? value : "", 1);
        free(env_backup[i]);
    }

    printf("\nAll tests passed!\n");
    return 0;
}