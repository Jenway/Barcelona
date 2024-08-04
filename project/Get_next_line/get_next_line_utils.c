#include "get_next_line.h"
// Helper function to concatenate two strings

char* ft_strnjoin(char* s1, char* s2, size_t len1, size_t len2)
{
    char* result = (char*)malloc(len1 + len2 + 1);
    if (!result)
        return NULL;
    if (s1)
        memcpy(result, s1, len1);
    if (s2)
        memcpy(result + len1, s2, len2);
    result[len1 + len2] = '\0';
    return result;
}

// Helper function to find the first occurrence of a character in a string
int ft_strnchr(char* s, char c, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        if (s[i] == c)
            return i;
    }
    return -1;
}
