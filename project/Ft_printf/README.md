# Ft_printf

一个自制的 `printf` 函数实现，符合 42 School 项目要求。该项目旨在深入理解格式化输出的工作原理，并实现一个功能相对完整的 `printf` 替代品。

## 项目概述

`ft_printf` 是对标准库 `printf` 函数的重新实现，支持多种格式说明符和格式化选项。通过手工实现这个函数，能够深入了解：

- 可变参数列表的处理（`va_list`）
- 格式字符串的解析
- 不同数据类型的格式化输出
- 内存管理和字符串操作

## 支持的格式说明符

| 格式符 | 描述 | 示例 |
|--------|------|------|
| `%c` | 单个字符 | `ft_printf("%c", 'A')` → `A` |
| `%s` | 字符串 | `ft_printf("%s", "Hello")` → `Hello` |
| `%d` / `%i` | 有符号十进制整数 | `ft_printf("%d", 42)` → `42` |
| `%u` | 无符号十进制整数 | `ft_printf("%u", 42U)` → `42` |
| `%x` | 无符号十六进制（小写） | `ft_printf("%x", 255)` → `ff` |
| `%X` | 无符号十六进制（大写） | `ft_printf("%X", 255)` → `FF` |
| `%p` | 指针地址 | `ft_printf("%p", ptr)` → `0x7fff5f` |
| `%%` | 字面百分号 | `ft_printf("%%")` → `%` |

## 支持的格式化标志

- `#` : 替代形式（如十六进制前缀 `0x`）
- `0` : 零填充
- `-` : 左对齐
- `+` : 总是显示符号
- ` ` : 正数前显示空格
- 宽度指定（如 `%10d`）
- 精度指定（如 `%.2f`）

## 编译与使用

```bash
# 编译静态库
make

# 清理对象文件
make clean

# 完全清理
make fclean

# 重新编译
make re
```

编译后会生成 `libftprintf.a` 静态库，可以在其他项目中链接使用：

```c
#include "ft_printf.h"

int main(void)
{
    ft_printf("Hello %s! Number: %d\n", "World", 42);
    return (0);
}
```

编译时链接库：
```bash
gcc main.c libftprintf.a -o program
```

## 项目结构

```
Ft_printf/
├── ft_printf.h          # 头文件，函数声明和宏定义
├── Makefile            # 编译配置
├── src/                # 源代码目录
│   ├── ft_printf.c         # 主函数入口
│   ├── ft_vprintf.c        # 核心实现函数
│   ├── handle_format_specifier.c  # 格式说明符处理
│   ├── print_formatted_unsigned.c # 无符号数格式化
│   ├── print_formatted_string.c   # 字符串格式化
│   └── 其他工具函数...
└── test/               # 测试代码
```

## 核心实现思路

### 1. 可变参数处理

使用 C 标准库的 `stdarg.h` 宏来处理可变参数：

```c
int ft_printf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int result = ft_vprintf(format, args);
    va_end(args);
    return result;
}
```

### 2. 格式字符串解析

通过状态机的方式解析格式字符串：
1. **普通字符**：直接输出
2. **%开始**：进入格式解析状态
3. **标志解析**：识别 `#`, `0`, `-`, `+`, ` ` 等标志
4. **宽度/精度解析**：提取数字或 `*` 参数
5. **说明符处理**：根据最终的格式符调用相应处理函数

### 3. 数字转换算法

实现了高效的数字到字符串转换：

```c
// 递归方式输出数字（避免使用额外内存）
int ft_putnunbr(unsigned long n, unsigned long base, int uppercase, int len)
{
    if (n >= base)
        len = ft_putnunbr(n / base, base, uppercase, len);
    
    char digit = (n % base < 10) ? 
                 '0' + (n % base) : 
                 (uppercase ? 'A' : 'a') + (n % base - 10);
    
    return len + ft_putchar(digit);
}
```

## 特色功能

- **内存高效**：最小化动态内存分配
- **模块化设计**：每个格式符都有独立的处理函数
- **标志组合**：支持多种格式标志的组合使用
- **错误处理**：对无效格式和边界情况进行适当处理

## 性能考虑

- 使用递归输出避免额外的字符串缓冲
- 位操作标志管理提高效率
- 最小化系统调用（write）次数

---

这个项目不仅是对 C 语言深度特性的探索，也是对格式化输出机制的完整实现，为后续更复杂的系统编程打下了坚实基础。