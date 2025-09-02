# Pipex

一个 Unix 管道机制的实现，模拟 shell 中的管道操作。该项目实现了类似 `< file1 cmd1 | cmd2 > file2` 的功能，深入理解进程间通信、文件重定向和系统调用。

## 项目概述

`pipex` 项目旨在重现 shell 管道的核心功能，通过实际编码来深入理解：

- **进程创建与管理**（`fork`, `exec`）
- **管道通信**（`pipe`）
- **文件描述符操作**（重定向）
- **错误处理与资源清理**
- **Here document** 支持（Bonus）

### 功能对比

```bash
# Shell 原生管道
< infile cmd1 | cmd2 > outfile

# 本项目实现
./pipex infile "cmd1" "cmd2" outfile

# Here document 模式 (Bonus)
cmd1 << LIMITER | cmd2 >> outfile
./pipex here_doc LIMITER "cmd1" "cmd2" outfile
```

## 编译与使用

```bash
# 编译
make

# 基础用法
./pipex file1 "ls -l" "wc -l" file2
# 等价于: < file1 ls -l | wc -l > file2

# Here document 模式
./pipex here_doc EOF "grep hello" "wc -l" outfile
# 等价于: grep hello << EOF | wc -l >> outfile

# 多命令链式管道（如果支持）
./pipex infile "cat" "grep pattern" "sort" "uniq" outfile
```

### 实际示例

```bash
# 示例 1：统计文件行数
./pipex /etc/passwd "cat" "wc -l" linecount.txt

# 示例 2：文本处理管道
./pipex input.txt "grep 'error'" "sort | uniq -c" errors.txt

# 示例 3：Here document
./pipex here_doc END "cat -n" "tail -5" output.txt
# 然后可以输入多行文本，以 'END' 结尾
```

## 核心数据结构

### 命令结构

```c
typedef struct s_cmd
{
    char            **argv;      // 参数数组，如 ["ls", "-l", NULL]
    char            *path;       // 可执行路径，如 "/bin/ls"
    int             pipe_fd[2];  // 与下一个命令的管道
    struct s_cmd    *next;       // 链表中的下一个命令
} t_cmd;
```

### 管道执行器结构

```c
typedef struct s_pipex
{
    int     is_here_doc;    // 是否是 here_doc 模式
    char    *infile;        // 输入文件名
    char    *outfile;       // 输出文件名
    char    *limiter;       // here_doc 的分界符
    t_cmd   *cmds;          // 命令链表
    char    **envp;         // 环境变量
} t_pipex;
```

## 核心实现逻辑

### 1. 管道执行流程

```c
int execute_pipeline(t_pipex *px)
{
    int infile_fd = open_infile(px->infile);
    int outfile_fd = open_outfile(px->outfile, px->is_here_doc);
    
    t_cmd *current = px->cmds;
    pid_t *pids = malloc(sizeof(pid_t) * cmd_count);
    
    // 为每个命令创建进程
    while (current) {
        if (current->next)
            safe_pipe(current->pipe_fd);  // 创建管道
        
        pid_t pid = safe_fork();
        if (pid == 0) {
            // 子进程：设置输入/输出重定向
            setup_redirections(current, infile_fd, outfile_fd);
            execve(current->path, current->argv, px->envp);
            perror_exit("execve failed");
        }
        
        pids[i++] = pid;
        current = current->next;
    }
    
    // 等待所有子进程完成
    return wait_for_children(pids, cmd_count);
}
```

### 2. 文件描述符重定向

```c
void setup_redirections(t_cmd *cmd, int infile_fd, int outfile_fd)
{
    // 设置输入重定向
    if (cmd == first_cmd) {
        dup2(infile_fd, STDIN_FILENO);
        close(infile_fd);
    } else {
        dup2(prev_cmd->pipe_fd[0], STDIN_FILENO);
        close(prev_cmd->pipe_fd[0]);
    }
    
    // 设置输出重定向
    if (cmd->next == NULL) {
        dup2(outfile_fd, STDOUT_FILENO);
        close(outfile_fd);
    } else {
        dup2(cmd->pipe_fd[1], STDOUT_FILENO);
        close(cmd->pipe_fd[1]);
    }
}
```

### 3. 命令路径解析

```c
char *get_cmd_path(char *cmd, char **envp)
{
    // 如果命令包含 '/'，直接使用
    if (strchr(cmd, '/'))
        return (access(cmd, X_OK) == 0) ? strdup(cmd) : NULL;
    
    // 在 PATH 环境变量中查找
    char *path_env = get_env_value("PATH", envp);
    if (!path_env)
        return NULL;
    
    char **paths = ft_split(path_env, ':');
    for (int i = 0; paths[i]; i++) {
        char *full_path = join_path(paths[i], cmd);
        if (access(full_path, X_OK) == 0) {
            free_split(paths);
            return full_path;
        }
        free(full_path);
    }
    
    free_split(paths);
    return NULL;
}
```

### 4. Here Document 实现

```c
int handle_here_doc(const char *limiter)
{
    int pipe_fd[2];
    safe_pipe(pipe_fd);
    
    pid_t pid = safe_fork();
    if (pid == 0) {
        // 子进程：读取用户输入直到遇到限制符
        close(pipe_fd[0]);
        
        char *line;
        while ((line = get_next_line(STDIN_FILENO))) {
            if (strcmp(line, limiter) == 0) {
                free(line);
                break;
            }
            write(pipe_fd[1], line, strlen(line));
            free(line);
        }
        
        close(pipe_fd[1]);
        exit(0);
    }
    
    close(pipe_fd[1]);
    return pipe_fd[0];  // 返回读端供后续使用
}
```

## 错误处理与资源管理

### 1. 安全系统调用包装

```c
pid_t safe_fork(void)
{
    pid_t pid = fork();
    if (pid == -1)
        perror_exit("fork failed");
    return pid;
}

void safe_pipe(int pipe_fd[2])
{
    if (pipe(pipe_fd) == -1)
        perror_exit("pipe failed");
}
```

### 2. 内存清理

```c
void free_cmd_list(t_cmd *cmd_list)
{
    while (cmd_list) {
        t_cmd *next = cmd_list->next;
        
        // 释放参数数组
        for (int i = 0; cmd_list->argv[i]; i++)
            free(cmd_list->argv[i]);
        free(cmd_list->argv);
        
        // 释放路径
        free(cmd_list->path);
        
        // 释放节点
        free(cmd_list);
        cmd_list = next;
    }
}
```

## 项目结构

```
pipex/
├── pipex.h              # 头文件和结构定义
├── Makefile            # 编译配置
├── src/
│   ├── main.c              # 程序入口
│   ├── init_pipex.c        # 初始化和参数解析
│   ├── parser.c            # 命令解析
│   ├── executor.c          # 管道执行逻辑
│   ├── path_utils.c        # 路径查找工具
│   ├── utils.c             # 通用工具函数
│   └── print_cmd_list.c    # 调试输出函数
├── 42_pipex_tester/    # 测试框架
└── pipex-tester/       # 另一个测试工具
```

## 高级特性

### 1. 多命令支持

支持超过两个命令的管道链：

```bash
./pipex input.txt "cmd1" "cmd2" "cmd3" "cmd4" output.txt
# 等价于: < input.txt cmd1 | cmd2 | cmd3 | cmd4 > output.txt
```

### 2. 复杂命令解析

支持带参数的复杂命令：

```bash
./pipex input.txt "grep -i error" "sort -r" "head -10" output.txt
```

### 3. 环境变量传递

正确传递和使用环境变量：

```c
execve(cmd->path, cmd->argv, envp);  // 传递完整环境
```

## 测试与验证

### 1. 功能测试

```bash
# 基础管道测试
echo "hello world" | ./pipex /dev/stdin "cat" "wc -w" /dev/stdout

# 文件操作测试
./pipex /etc/passwd "head -10" "tail -5" result.txt
cat result.txt

# 错误处理测试
./pipex nonexistent.txt "cat" "wc" output.txt  # 应正确处理文件不存在
```

### 2. 内存泄漏检测

```bash
valgrind --leak-check=full ./pipex input.txt "cat" "wc" output.txt
```

## 学习要点

1. **进程模型**：理解 fork/exec 模型和进程生命周期
2. **文件描述符**：掌握 dup2 和文件描述符继承机制
3. **管道通信**：理解管道的读写端和缓冲机制
4. **错误处理**：系统调用失败时的优雅处理
5. **资源管理**：避免文件描述符泄漏和内存泄漏

---

`pipex` 项目是系统编程的精华体现，它将理论知识转化为实际可运行的代码，为深入理解 Unix 系统的工作原理奠定了坚实基础。