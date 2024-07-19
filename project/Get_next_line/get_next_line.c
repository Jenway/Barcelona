#include "get_next_line.h" // Include the header file and declare the function prototype

static char* get_line_helper(int fd, char* buffer, int* seek, int* eof)
{
    char* line = NULL;
    size_t line_len = 0;

    while (1) {
        if (*seek >= BUFFER_SIZE || buffer[*seek] == '\0') {
            int bytes_read = read(fd, buffer, BUFFER_SIZE);
            if (bytes_read <= 0) {
                *eof = 1;
                break;
            }
            buffer[bytes_read] = '\0';
            *seek = 0;
        }

        int index = ft_strnchr(buffer + *seek, '\n', BUFFER_SIZE - *seek);
        size_t chunk_len = (index == -1) ? strlen(buffer + *seek) : index + 1U;

        char* temp = ft_strjoin(line, buffer + *seek, line_len, chunk_len);
        free(line);
        line = temp;
        line_len += chunk_len;
        *seek += chunk_len;

        if (index != -1)
            break;
    }

    return line;
}

char* get_next_line(int fd)
{
    static char buffer[BUFFER_SIZE + 1];
    static int seek = 0;
    static int eof = 0;

    if (eof) {
        return NULL;
    }
    return get_line_helper(fd, buffer, &seek, &eof);
}