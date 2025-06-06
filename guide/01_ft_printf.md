ft_printf

Because ft_putnbr() and ft_putstr() aren’t enough

Summary:

- The goal of this project is pretty straightforward. You will recode printf().
- You will mainly learn about using a variable number of arguments. How cool is that??
- It is actually pretty cool `:)`

Version: 10

## Chapter I Introduction

You will discover a popular and versatile C function: `printf()`. This exercise is a great opportunity to improve your programming skills. It is of moderate difficulty.

You will discover `variadic functions` in C.

The key to a successful ft_printf is a well-structured and extensible code.

> [!NOTE]
> Once this assignment passed, you will be allowed to add your ft_printf() to your libft so you can use it in your school C projects.

## Chapter II Common Instructions

same as [00_libft.md](00_libft.md#chapter-ii-common-instructions)

## Chapter III Mandatory part

| Program name | `libftprintf.a` |
|--------------|-----------------|
| Turn in files | `Makefile`, `*.h`, `*/*.h`, `*.c`, `*/*.c` |
| External functs. | `NAME`, `all`, `clean`, `fclean`, `re`, `malloc`, `free`, `write`, `va_start`, `va_arg`, `va_copy`, `va_end` |
| Libft authorized | `Yes` |
| Description | Write a library that contains `ft_printf()`, a function that will mimic the original `printf()`. |

You have to recode the printf() function from libc.

The prototype of ft_printf() is:

`int ft_printf(const char *, ...);`

Here are the requirements:

- Don’t implement the buffer management of the original printf().
- Your function has to handle the following conversions: `cspdiuxX%`
- Your function will be compared against the original `printf()`.
- You must use the command `ar` to create your library.
  - Using the libtool command is forbidden.
- Your libftprintf.a has to be created at the root of your repository.

You have to implement the following conversions:

- %c Prints a single character.
- %s Prints a string (as defined by the common C convention).
- %p The void * pointer argument has to be printed in hexadecimal format.
- %d Prints a decimal (base 10) number.
- %i Prints an integer in base 10.
- %u Prints an unsigned decimal (base 10) number.
- %x Prints a number in hexadecimal (base 16) lowercase format.
- %X Prints a number in hexadecimal (base 16) uppercase format.
- %% Prints a percent sign.

## Chapter IV Bonus part

You don’t have to do all the bonuses.

Bonus list:

- Manage any combination of the following flags: ’-0.’ and the field minimum width under all conversions.

- Manage all the following flags: ’# +’ (Yes, one of them is a space)

> [!NOTE]
> If you plan to complete the bonus part, think about the implementation of your extra features from the start.This way,you will avoid the pitfalls of a naive approach.


> [!WARNING]
> The bonus part will only be assessed if the mandatory part is PERFECT. Perfect means the mandatory part has been integrally done and works without malfunctioning.If you have not passed ALL the mandatory requirements, your bonus part will not be evaluated at all.


## Chapter V Submission and peer-evaluation

Turn in your assignment in your Git repository as usual. Only the work inside your repository will be evaluated during the defense. Don’t hesitate to double check the names of your files to ensure they are correct.

Once this assignment passed, you will be allowed to add your ft_printf() to your libft so you can use it in your school C projects.

``` brainfuck
++++++++++[>+>+++>+++++++>++++++++++<<<<-]>>>.>---.++++++++++++.++.+++
+++.--.<<++.>>------.------------.+++++++++++++.<<.>>++++++.------------
.-------. +++++++++++++++++++.<<.>>----------------.+++++.+++++++++.---
----------.--.+ ++++++++++++++++.--------.+++++++++++++.<<.>>----------
-------------.+++.+++ ++++.---.----.+++++++++++++++++.---------------
--.-.<<.>>+++++.+++++.<<.>-------...
```