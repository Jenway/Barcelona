# Get_next_line

一个高效的逐行读取函数，能够从文件描述符中一次读取一行，支持任意长度的行和多种缓冲区大小。这是 42 School 的经典项目，旨在深入理解文件 I/O、缓冲机制和内存管理。

## 项目概述

`get_next_line` 函数解决了一个看似简单却颇具挑战性的问题：**如何从文件描述符中高效地读取一行文本**。

该函数的核心挑战包括：
- **缓冲区管理**：如何处理跨越多次 `read()` 调用的行
- **内存效率**：在不知道行长度的情况下动态分配内存
- **边界处理**：文件结尾、空行、超长行等边界情况
- **多文件支持**：同时处理多个文件描述符（bonus）

## 函数签名

```c
char *get_next_line(int fd);
```

- **参数**：`fd` - 要读取的文件描述符
- **返回值**：
  - 成功时返回包含换行符的一行文本
  - 文件读取完毕时返回 `NULL`
  - 错误时返回 `NULL`

## 编译与使用

### 基础版本

```bash
# 编译，设置缓冲区大小为 42
gcc -Wall -Wextra -Werror -D BUFFER_SIZE=42 get_next_line.c get_next_line_utils.c main.c -o gnl

# 使用不同的缓冲区大小测试
gcc -D BUFFER_SIZE=1 get_next_line.c get_next_line_utils.c main.c -o gnl_small
gcc -D BUFFER_SIZE=1024 get_next_line.c get_next_line_utils.c main.c -o gnl_large
```

### Bonus 版本（多文件描述符支持）

```bash
gcc -D BUFFER_SIZE=42 get_next_line_bonus.c get_next_line_utils_bonus.c main.c -o gnl_bonus
```

### 示例用法

```c
#include "get_next_line.h"
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc != 2)
        return (1);
    
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1)
        return (1);
    
    char *line;
    while ((line = get_next_line(fd)) != NULL)
    {
        printf("Read: %s", line);
        free(line);
    }
    
    close(fd);
    return (0);
}
```

## 核心算法与实现

### 1. 缓冲区机制

函数维护一个静态缓冲区，在多次调用之间保持状态：

```c
static char* get_line_helper(int fd, char* buffer, int* seek, int* eof)
{
    char* line = NULL;
    size_t line_len = 0;

    while (1) {
        // 如果缓冲区读完或为空，读取新数据
        if (*seek >= BUFFER_SIZE || buffer[*seek] == '\0') {
            int bytes_read = read(fd, buffer, BUFFER_SIZE);
            if (bytes_read <= 0) {
                *eof = 1;
                break;
            }
            buffer[bytes_read] = '\0';
            *seek = 0;
        }
        
        // 在缓冲区中查找换行符
        int index = ft_strnchr(buffer + *seek, '\n', BUFFER_SIZE - *seek);
        size_t chunk_len = (index == -1) ? strlen(buffer + *seek) : index + 1;
        
        // 将数据追加到结果字符串
        char* temp = ft_strnjoin(line, buffer + *seek, line_len, chunk_len);
        free(line);
        line = temp;
        line_len += chunk_len;
        *seek += chunk_len;
        
        // 如果找到换行符，结束循环
        if (index != -1)
            break;
    }
    
    return line;
}
```

### 2. 动态字符串拼接

`ft_strnjoin` 函数负责高效地拼接字符串片段：

```c
char* ft_strnjoin(char* s1, char* s2, size_t len1, size_t len2)
{
    if (!s2 || len2 == 0)
        return s1;
    
    char* result = malloc(len1 + len2 + 1);
    if (!result)
        return NULL;
    
    if (s1 && len1 > 0)
        memcpy(result, s1, len1);
    
    memcpy(result + len1, s2, len2);
    result[len1 + len2] = '\0';
    
    return result;
}
```

### 3. 换行符查找

优化的字符查找函数，限制搜索范围：

```c
int ft_strnchr(char* s, char c, size_t n)
{
    size_t i = 0;
    while (i < n && s[i] != '\0')
    {
        if (s[i] == c)
            return (int)i;
        i++;
    }
    return -1;
}
```

## 关键设计决策

### 1. 缓冲区大小

`BUFFER_SIZE` 是编译时常量，影响性能和内存使用：

- **小缓冲区**（如 1）：更多 `read()` 系统调用，但内存占用小
- **大缓冲区**（如 4096）：更少系统调用，但可能浪费内存
- **推荐值**：32-1024，在性能和内存之间取得平衡

### 2. 内存管理策略

- **最小分配**：只分配当前所需的内存
- **及时释放**：每次拼接后释放旧字符串
- **错误处理**：malloc 失败时的优雅处理

### 3. 多文件描述符支持（Bonus）

```c
// 使用文件描述符作为索引的静态数组
static char* saved_buffers[FD_MAX];

char *get_next_line(int fd)
{
    if (fd < 0 || fd >= FD_MAX || BUFFER_SIZE <= 0)
        return NULL;
    
    if (!saved_buffers[fd])
        saved_buffers[fd] = malloc(BUFFER_SIZE + 1);
    
    // ... 使用 saved_buffers[fd] 作为该 fd 的专用缓冲区
}
```

## 项目结构

```
Get_next_line/
├── get_next_line.h              # 头文件
├── get_next_line.c              # 主函数实现
├── get_next_line_utils.c        # 工具函数
├── get_next_line_bonus.h        # Bonus 头文件
├── get_next_line_bonus.c        # Bonus 实现（多 fd 支持）
├── get_next_line_utils_bonus.c  # Bonus 工具函数
├── Makefile                     # 编译配置
└── gnl.mk                       # 构建配置
```

## 测试用例

该函数应当能够正确处理：

1. **正常文件**：包含多行文本的普通文件
2. **空文件**：零字节文件
3. **单行文件**：没有换行符的文件
4. **超长行**：长度超过缓冲区大小的行
5. **标准输入**：从 `stdin` (fd=0) 读取
6. **二进制文件**：包含 null 字节的文件
7. **网络套接字**：从网络连接读取数据

## 性能特性

- **时间复杂度**：O(n)，其中 n 是读取的字符总数
- **空间复杂度**：O(m)，其中 m 是当前行的长度
- **系统调用次数**：最优情况下为 ⌈文件大小/BUFFER_SIZE⌉

---

`get_next_line` 项目是理解底层文件操作和内存管理的绝佳练习，它展示了如何在资源受限的环境下编写高效且健壮的 C 代码。