# Minishell

一个功能完备的 Shell 解释器实现，支持命令解析、管道、重定向、内置命令等核心功能。这是 42 School 的重要项目，旨在深入理解 Shell 的工作原理和系统编程。

## 项目概述

`minishell` 实现了一个类似 bash 的命令行解释器，包含了现代 Shell 的核心功能：

- **命令解析**：词法分析 → 语法分析 → 抽象语法树
- **变量扩展**：环境变量、特殊变量（`$?`, `$$`）
- **引号处理**：单引号、双引号的不同语义
- **重定向**：输入/输出重定向、追加重定向、Here Document
- **管道**：多命令管道链
- **内置命令**：`echo`, `cd`, `pwd`, `export`, `unset`, `env`, `exit`
- **信号处理**：`Ctrl+C`, `Ctrl+D`, `Ctrl+\` 的正确处理

## 编译与运行

```bash
# 编译
make

# 启动交互式 shell
./minishell

# 非交互式执行（用于测试）
echo "ls -la | grep total" | ./minishell
```

### 基本使用示例

```bash
# 启动 minishell
$ ./minishell
minishell$ echo "Hello World"
Hello World

# 变量操作
minishell$ export MY_VAR="test"
minishell$ echo $MY_VAR
test

# 管道和重定向
minishell$ ls -la | grep "\.c" > c_files.txt
minishell$ cat c_files.txt

# Here document
minishell$ cat << EOF > output.txt
> Line 1
> Line 2
> EOF

# 内置命令
minishell$ cd /tmp
minishell$ pwd
/tmp
minishell$ exit
```

## 架构设计

### 整体流程

```
输入字符串 → 词法分析 → 语法分析 → 变量扩展 → 执行
     ↓           ↓          ↓           ↓        ↓
   "ls | wc"   Token链   命令链     扩展后链   执行结果
```

### 1. 前端处理（Frontend）

#### 词法分析器（Tokenizer）

将输入字符串分解为 token 序列：

```c
typedef enum e_token_type {
    TOKEN_WORD,         // 普通词汇：ls, -la, filename
    TOKEN_VARIABLE,     // 变量：$VAR, $?
    TOKEN_PIPE,         // 管道：|
    TOKEN_REDIR_IN,     // 输入重定向：<
    TOKEN_REDIR_OUT,    // 输出重定向：>
    TOKEN_REDIR_APPEND, // 追加重定向：>>
    TOKEN_HEREDOC,      // Here document：<<
    TOKEN_SQUOTE,       // 单引号：'
    TOKEN_DQUOTE,       // 双引号："
    TOKEN_SPACE,        // 空白字符
    TOKEN_EOF           // 输入结束
} t_token_type;
```

**示例转换**：
```bash
"ls -la | grep 'file' > output.txt"
↓
[WORD:ls] [SPACE] [WORD:-la] [SPACE] [PIPE] [SPACE] [WORD:grep] [SPACE] 
[SQUOTE] [WORD:file] [SQUOTE] [SPACE] [REDIR_OUT] [SPACE] [WORD:output.txt] [EOF]
```

#### 语法分析器（Parser）

将 token 序列转换为命令链表：

```c
typedef struct s_cmd {
    t_argument* args;       // 命令和参数
    t_redirect* in_redir;   // 输入重定向链表
    t_redirect* out_redir;  // 输出重定向链表
    char** argv;            // 扩展后的参数数组
    char* path;             // 可执行文件路径
    int is_builtin;         // 是否为内置命令
    struct s_cmd* next;     // 管道中的下一个命令
} t_cmd;
```

#### 变量扩展器（Expander）

处理变量替换和引号解析：

```c
// 变量扩展规则
// $VAR     → 环境变量 VAR 的值
// $?       → 上一个命令的退出码
// $$       → 当前进程 PID
// '$VAR'   → 字面量 $VAR（单引号内不扩展）
// "$VAR"   → 扩展为变量值（双引号内扩展）

void expand_commands(t_shell* sh)
{
    for (t_cmd* cmd = sh->cmds; cmd; cmd = cmd->next) {
        cmd->argv = expand_arguments(cmd->args, sh);
        cmd->path = find_command_path(cmd->argv[0], sh->envp);
        cmd->is_builtin = is_builtin(cmd->argv[0]);
    }
}
```

### 2. 后端执行（Backend）

#### 执行器（Executor）

负责创建进程、设置管道和重定向：

```c
int execute_cmd_chain(t_shell* sh)
{
    if (!sh->cmds)
        return 0;
    
    // 单命令优化
    if (!sh->cmds->next) {
        if (sh->cmds->is_builtin)
            return exec_builtin(sh->cmds, sh);
        else
            return exec_single_command(sh->cmds, sh);
    }
    
    // 多命令管道
    return exec_pipeline(sh->cmds, sh);
}
```

#### 重定向处理器（Redirect Handler）

```c
typedef enum e_redirect_type {
    REDIR_IN,       // < file
    REDIR_OUT,      // > file
    REDIR_APPEND,   // >> file  
    REDIR_HEREDOC   // << limiter
} t_redirect_type;

int setup_redirections(t_cmd* cmd)
{
    // 处理输入重定向
    for (t_redirect* redir = cmd->in_redir; redir; redir = redir->next) {
        int fd = open_input_redirect(redir);
        if (fd == -1) return -1;
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    
    // 处理输出重定向  
    for (t_redirect* redir = cmd->out_redir; redir; redir = redir->next) {
        int fd = open_output_redirect(redir);
        if (fd == -1) return -1;
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    
    return 0;
}
```

#### 内置命令处理器（Builtins）

```c
static t_builtin builtins[] = {
    {"echo", builtin_echo},
    {"cd", builtin_cd},
    {"pwd", builtin_pwd},
    {"export", builtin_export},
    {"unset", builtin_unset},
    {"env", builtin_env},
    {"exit", builtin_exit},
    {NULL, NULL}
};

int exec_builtin(t_cmd* cmd, t_shell* sh)
{
    for (int i = 0; builtins[i].name; i++) {
        if (strcmp(cmd->argv[0], builtins[i].name) == 0)
            return builtins[i].func(cmd->argv, sh);
    }
    return 127;  // Command not found
}
```

## 核心特性详解

### 1. 引号处理

```bash
# 单引号：所有字符字面量化
minishell$ echo '$HOME is $HOME'
$HOME is $HOME

# 双引号：变量扩展，但保护空格
minishell$ echo "$HOME is home"
/Users/john is home

# 混合引号
minishell$ echo "Hello '$USER'"
Hello 'john'
```

### 2. 变量扩展

```bash
# 环境变量
minishell$ echo $HOME
/Users/john

# 退出码
minishell$ false
minishell$ echo $?
1

# 变量组合
minishell$ echo $HOME/Documents
/Users/john/Documents
```

### 3. Here Document

```bash
minishell$ cat << EOF
> This is line 1
> This is line 2
> EOF
This is line 1
This is line 2
```

### 4. 复杂管道

```bash
# 多级管道
minishell$ ls -la | grep "\.c$" | wc -l

# 管道 + 重定向
minishell$ cat file.txt | grep pattern | sort > sorted.txt
```

### 5. 信号处理

- **Ctrl+C (SIGINT)**：中断当前命令，显示新提示符
- **Ctrl+D (EOF)**：优雅退出 shell
- **Ctrl+\\ (SIGQUIT)**：在命令执行时显示 "Quit: 3"

## 项目结构

```
minishell/
├── include/
│   ├── minishell.h      # 主头文件
│   └── executor.h       # 执行器相关定义
├── src/
│   ├── frontend/        # 前端处理
│   │   ├── tokenizer.c      # 词法分析
│   │   ├── parser.c         # 语法分析
│   │   └── expander.c       # 变量扩展
│   ├── backend/         # 后端执行
│   │   ├── executor.c       # 主执行逻辑
│   │   ├── builtins.c       # 内置命令
│   │   └── redirect.c       # 重定向处理
│   ├── repl.c           # 交互式读取-求值-打印循环
│   ├── shell.c          # Shell 主逻辑
│   └── utils.c          # 工具函数
├── test/                # 测试用例
├── Makefile            # 编译配置
└── config.mk           # 构建配置
```

## 技术难点与解决方案

### 1. 词法分析中的引号状态机

```c
typedef enum e_quote_state {
    QUOTE_NONE,     // 非引号状态
    QUOTE_SINGLE,   // 单引号状态
    QUOTE_DOUBLE    // 双引号状态
} t_quote_state;

// 在 tokenizer 中维护引号状态，正确处理嵌套和转义
```

### 2. Here Document 的实现

```c
int handle_heredoc(const char* limiter)
{
    int pipe_fd[2];
    pipe(pipe_fd);
    
    // 创建子进程读取用户输入
    if (fork() == 0) {
        close(pipe_fd[0]);
        char* line;
        while ((line = readline("heredoc> "))) {
            if (strcmp(line, limiter) == 0) {
                free(line);
                break;
            }
            write(pipe_fd[1], line, strlen(line));
            write(pipe_fd[1], "\n", 1);
            free(line);
        }
        close(pipe_fd[1]);
        exit(0);
    }
    
    close(pipe_fd[1]);
    return pipe_fd[0];
}
```

### 3. 内置命令的环境变量修改

```c
// 内置命令需要在父进程中执行，以便修改 shell 环境
int builtin_export(char** argv, t_shell* sh)
{
    if (!argv[1]) {
        print_exported_vars(sh->envp);
        return 0;
    }
    
    for (int i = 1; argv[i]; i++) {
        if (parse_assignment(argv[i])) {
            sh->envp = update_env(sh->envp, argv[i]);
        } else {
            printf("export: invalid assignment\n");
            return 1;
        }
    }
    
    return 0;
}
```

## 测试与验证

### 1. 单元测试

```bash
# 测试词法分析器
echo "ls | wc" | ./test_tokenizer

# 测试语法分析器  
echo "ls -la > file" | ./test_parser

# 测试变量扩展
echo 'echo $HOME' | ./test_expander
```

### 2. 集成测试

```bash
# 与 bash 行为对比测试
./minishell_tester.sh

# 内存泄漏检测
valgrind --leak-check=full ./minishell
```

### 3. 压力测试

```bash
# 深层嵌套管道
echo "cat /dev/zero | head -1000 | wc -l | cat | cat | cat" | ./minishell

# 大量重定向
echo "echo hello > a > b > c > d > e" | ./minishell
```

## 学习要点

1. **编译原理基础**：词法分析、语法分析的实际应用
2. **系统编程**：进程管理、信号处理、文件描述符操作
3. **内存管理**：复杂数据结构的内存分配与释放
4. **错误处理**：健壮的错误检测和恢复机制
5. **用户体验**：交互式程序的设计考虑

---

`minishell` 项目是系统编程和编译原理的综合实践，通过实现一个完整的 Shell 解释器，深入理解了操作系统、编译器和用户界面设计的核心概念。