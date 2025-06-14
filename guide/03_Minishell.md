Minishell

As beautiful as a shell

Summary:

- This project is about creating a simple shell.
- Yes, your own little bash.
- You will learn a lot about processes and file descriptors.

Version: 7.1

## Chapter I Introduction

The existence of shells is linked to the very existence of IT.

At the time, all developers agreed that communicating with a computer using aligned 1/0 switches was seriously irritating.

It was only logical that they came up with the idea of creating a software to communicate with a computer using interactive lines of commands in a language somewhat
close to the human language.

Thanks to Minishell, you’ll be able to travel through time and come back to problems people faced when Windows didn’t exist.

## Chapter III Mandatory part

以下是将你提供的 `minishell` 项目信息转化为表格后的形式：

| **项**                | **内容**                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| --------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Program name**      | `minishell`                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| **Turn in files**     | `Makefile`, `*.h`, `*.c`                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| **Makefile 必须包含** | `NAME`, `all`, `clean`, `fclean`, `re`                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| **Arguments**         | *无特别说明*                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| **允许的外部函数**    | `readline`, `rl_clear_history`, `rl_on_new_line`, `rl_replace_line`, `rl_redisplay`, `add_history`,<br>`printf`, `malloc`, `free`, `write`,<br>`access`, `open`, `read`, `close`, `fork`, `wait`, `waitpid`, `wait3`, `wait4`,<br>`signal`, `sigaction`, `sigemptyset`, `sigaddset`, `kill`, `exit`, `getcwd`, `chdir`,<br>`stat`, `lstat`, `fstat`, `unlink`, `execve`, `dup`, `dup2`, `pipe`,<br>`opendir`, `readdir`, `closedir`, `strerror`, `perror`, `isatty`,<br>`ttyname`, `ttyslot`, `ioctl`, `getenv`, `tcsetattr`, `tcgetattr`,<br>`tgetent`, `tgetflag`, `tgetnum`, `tgetstr`, `tgoto`, `tputs` |
| **是否允许 Libft**    | 是（Yes）                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| **项目描述**          | 编写一个 shell 程序                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |


Your shell should:

- Display a prompt when waiting for a new command.
- Have a working history.
- Search and launch the right executable (based on the PATH variable or using a relative or an absolute path).
- Avoid using more than one global variable to indicate a received signal. Consider the implications: this approach ensures that your signal handler will not access your main data structures.

> [!WARNING]
> Be careful. This global variable cannot provide any other information or data access than the number of a received signal.
> Therefore, using "norm" type structures in the global scope is forbidden.

- Not interpret unclosed quotes or special characters which are not required by the subject such as \ (backslash) or ; (semicolon).
- Handle ’ (single quote) which should prevent the shell from interpreting the metacharacters in the quoted sequence.
- Handle " (double quote) which should prevent the shell from interpreting the metacharacters in the quoted sequence except for $ (dollar sign).
- Implement redirections:
    - < should redirect input.
    - > should redirect output.
    - << should be given a delimiter, then read the input until a line containing the delimiter is seen. However, it doesn’t have to update the history!
    - >> should redirect output in append mode.
- Implement pipes (| character). The output of each command in the pipeline is connected to the input of the next command via a pipe.
- Handle environment variables ($ followed by a sequence of characters) which should expand to their values.
- Handle $? which should expand to the exit status of the most recently executed foreground pipeline.
- Handle ctrl-C, ctrl-D and ctrl-\ which should behave like in bash.
- In interactive mode:
    - ctrl-C displays a new prompt on a new line.
    - ctrl-D exits the shell.
    - ctrl-\ does nothing.
- Your shell must implement the following builtins:
    - echo with option -n
    - cd with only a relative or absolute path
    - pwd with no options
    - export with no options
    - unset with no options
    - env with no options or arguments
    - exit with no options

The `readline()` function can cause memory leaks. You don’t have to fix them. But that doesn’t mean your own code, yes the code you wrote, can have memory leaks.

> [!NOTE]
> You should limit yourself to the subject description. Anything that is not asked is not required.
> 
> If you have any doubt about a requirement, take bash as a reference.

## Chapter IV Bonus part

Your program has to implement:

- && and || with parenthesis for priorities.
- Wildcards * should work for the current working directory.
