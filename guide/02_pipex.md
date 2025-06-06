Pipex

Summary:

This project will let you discover in detail a UNIX mechanism that you already know by using it in your program.

## Chapter I Foreword
Cristina: "Go dance salsa somewhere `:)`"

## Chapter II Common Instructions

same as [00_libft.md](00_libft.md#chapter-ii-common-instructions)

## Chapter III Mandatory part

| Program name     | pipex                                                                                                                                                                                                                                                |
| ---------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Turn in files    | Makefile, *.h, *.c                                                                                                                                                                                                                                   |
| Makefile         | NAME, all, clean, fclean, re                                                                                                                                                                                                                         |
| Arguments        | file1 cmd1 cmd2 file2                                                                                                                                                                                                                                |
| External functs. | - `open`, `close`, `read`, `write`, `malloc`, `free`, `perror`, `strerror`, `exit` <br> - All functions of the math library (-lm compiler option, man man 3 math) <br> - All functions of the MiniLibX <br> - ft_printf and any equivalent YOU coded |
| Libft authorized | Yes                                                                                                                                                                                                                                                  |
| Description      | This project is about handling pipes.                                                                                                                                                                                                                |

Your program will be executed as follows:

```c
./pipex file1 cmd1 cmd2 file2
```

It must take 4 arguments:

- file1 and file2 are file names.
- cmd1 and cmd2 are shell commands with their parameters.

It must behave exactly the same as the shell command below:

```c
$> < file1 cmd1 | cmd2 > file2
```

### III.1 Examples
```c
$> ./pipex infile "ls -l" "wc -l" outfile
```

Should behave like: `< infile ls -l | wc -l > outfile`

```c
$> ./pipex infile "grep a1" "wc -w" outfile
```

Should behave like: `< infile grep a1 | wc -w > outfile`

### III.2 Requirements

Your project must comply with the following rules:

- You have to turn in a Makefile which will compile your source files. It must not relink.
- You have to handle errors thoroughly. In no way your program should quit unexpectedly (segmentation fault, bus error, double free, and so forth).
- Your program mustn’t have memory leaks.
- If you have any doubt, handle the errors like the shell command:
  ```c
  < file1 cmd1 | cmd2 > file2
  ```

## Chapter IV Bonus part

You will get extra points if you:

- Handle multiple pipes.

  This:

  ``` BASH
  $> ./pipex file1 cmd1 cmd2 cmd3 ... cmdn file2
  ```

  Should behave like:

  ``` BASH
  < file1 cmd1 | cmd2 | cmd3 ... | cmdn > file2
  ```

- Support `<<` and `>>` when the first parameter is "here_doc".

  This:

  ``` BASH
  $> ./pipex here_doc LIMITER cmd cmd1 file
  ```

  Should behave like:

  ``` BASH
  cmd << LIMITER | cmd1 >> file
  ```
