// map_loader.c

#include "../FdF.h"
#include "libft.h"
#include <fcntl.h> // open
#include <stdlib.h> // malloc, free
#include <unistd.h> // close

/*
 * 逐行读取，将文件写入链表
 */
t_list* read_lines(const char* filename)
{
    int fd;
    char* line;
    t_list* lines = NULL;
    t_list* node;

    fd = open(filename, O_RDONLY);
    if (fd < 0)
        return (NULL);
    while ((line = get_next_line(fd)) != NULL) {
        // line 的资源管理权 move 到链表节点中，自然不用 free(line)
        node = ft_lstnew(line);
        if (!node) {
            free(line);
            ft_lstclear(&lines, free);
            close(fd);
            return (NULL);
        }
        ft_lstadd_back(&lines, node);
    }
    close(fd);
    return (lines);
}

/*
 * 辅助：计算以空格分隔的单词数（列数）
 */
static int count_columns(const char* s)
{
    int count = 0;
    int in_word = 0;
    while (*s) {
        if (*s != ' ' && !in_word) {
            in_word = 1;
            count++;
        } else if (*s == ' ')
            in_word = 0;
        s++;
    }
    return (count);
}


/*
 * 把链表每行解析成整数数组，再赋值给 t_map->points[y][x].z
 * x,y 坐标就是索引值，z 由 atoi 返回。
 */
t_map* parse_map(t_list* lines)
{
    int height = ft_lstsize(lines);
    int width = 0;
    t_map* map = malloc(sizeof(t_map));
    char** tokens;
    int y = 0;

    if (!map)
        return (NULL);
    map->height = height;
    map->points = malloc(sizeof(t_point*) * height);
    if (!map->points)
        return (free(map), NULL);

    for (t_list* cur = lines; cur; cur = cur->next, y++) {
        tokens = ft_split((char*)cur->content, ' ');
        if (!tokens)
            return (NULL);
        if (y == 0)
            width = count_columns(cur->content);
        // 每一行都应当有相同的列数，否则算作格式错误
        if (count_columns(cur->content) != width) {
            ft_lstclear(&lines, free);
            free_map(map);
            return (NULL);
        }
        map->width = width;
        map->points[y] = malloc(sizeof(t_point) * width);
        if (!map->points[y])
            return (NULL);
        for (int x = 0; x < width; x++) {
            map->points[y][x].x = x;
            map->points[y][x].y = y;
            map->points[y][x].z = ft_atoi(tokens[x]);
        }
        for (int i = 0; tokens[i]; i++)
            free(tokens[i]);
        free(tokens);
    }
    return (map);
}

void free_map(t_map* map)
{
    if (!map)
        return;
    for (int y = 0; y < map->height; y++)
        free(map->points[y]);
    free(map->points);
    free(map);
}