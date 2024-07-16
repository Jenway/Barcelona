#include "../libft.h"

// ft_split()
// Allocates (with malloc(3)) and returns an array
// of strings obtained by splitting ’s’ using the
// character ’c’ as a delimiter. The array must end
// with a NULL pointer.

static size_t count_words(char const* s, char c)
{
    size_t count = 0;
    size_t i = 0;
    while (s[i]) {
        while (s[i] && s[i] == c) {
            i++;
        }
        if (s[i]) {
            count++;
        }
        while (s[i] && s[i] != c) {
            i++;
        }
    }
    return count;
}

char** ft_split(char const* s, char c)
{
    if (!s) {
        return NULL;
    }

    size_t words = count_words(s, c);
    char** split = (char**)malloc((words + 1) * sizeof(char*));
    if (!split) {
        return NULL;
    }
    size_t i = 0;
    size_t j = 0;
    while (i < words) {
        while (s[j] && s[j] == c) {
            j++;
        }
        size_t start = j;
        while (s[j] && s[j] != c) {
            j++;
        }
        split[i] = ft_substr(s, start, j - start);
        if (!split[i]) {
            while (i > 0) {
                free(split[i - 1]);
                i--;
            }
            free(split);
            return NULL;
        }
        i++;
    }
    split[i] = NULL;
    return split;
}