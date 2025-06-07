#include "../stack.h"

// 工具：找栈中最大值
static int find_max_value(t_stack* a)
{
    t_node* cur = a->top;
    int mx = cur->value;
    while (cur) {
        if (cur->value > mx)
            mx = cur->value;
        cur = cur->next;
    }
    return mx;
}

static int find_min_value(t_stack* a)
{
    t_node* cur = a->top;
    int mn = cur->value;
    while (cur) {
        if (cur->value < mn)
            mn = cur->value;
        cur = cur->next;
    }
    return mn;
}

void radix_sort(t_stack* a, t_stack* b)
{
    if (a->size <= 1)
        return;

    // 1) 计算平移量
    int min = find_min_value(a);
    int shift = (min < 0) ? -min : 0;

    // 2) 确定平移后最大值与需要位数
    int max = find_max_value(a) + shift;
    int max_bits = 0;
    while ((max >> max_bits) != 0)
        max_bits++;

    // 3) 按位处理
    for (int i = 0; i < max_bits; i++) {
        int len = a->size;
        for (int j = 0; j < len; j++) {
            int key = a->top->value + shift;
            if (((key >> i) & 1) == 1)
                rotate(a); // (ra)
            else
                push(a, b); // (pb)
        }
        while (b->size > 0)
            push(b, a); // (pa)
    }
}