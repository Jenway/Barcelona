#if !defined(__GET__NEXT__LINE__H__)
#define __GET__NEXT__LINE__H__

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 32
#endif

void ft_bzero(void* s, size_t n);
char* ft_strjoin(char* s1, char* s2, size_t len1, size_t len2);
int ft_strnchr(char* s, char c, size_t n);
char* get_next_line(int fd);

#endif // __GET__NEXT__LINE__H__