#include "../libft.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void test_itoa(char* (*func)(int))
{
    char* ret1 = func(9);
    char* ans1 = "9";
    assert(strcmp(ret1, ans1) == 0);
    free(ret1);

    char* ret2 = func(-9);
    char* ans2 = "-9";
    assert(strcmp(ret2, ans2) == 0);
    free(ret2);

    char* ret3 = func(0);
    char* ans3 = "0";

    assert(strcmp(ret3, ans3) == 0);

    char* ret4 = func(1234567890);
    char* ans4 = "1234567890";
    assert(strcmp(ret4, ans4) == 0);
}
