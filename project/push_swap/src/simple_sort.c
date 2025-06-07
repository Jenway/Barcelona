#include "../stack.h"

static int min_pos(t_stack* a)
{
    t_node* curr = a->top;
    int min_pos = 0;
    int pos = 0;
    int min_value = curr->value;

    while (curr) {
        if (curr->value < min_value) {
            min_value = curr->value;
            min_pos = pos;
        }
        curr = curr->next;
        pos++;
    }
    return min_pos;
}

static void push_min_to_b(t_stack* a, t_stack* b)
{
    int pos = min_pos(a);
    // 如果最小值在前半段，用 rotate；否则用 reverse_rotate
    if (pos < a->size - pos) {
        while (pos-- > 0)
            rotate(a);
    } else {
        while (pos++ < a->size)
            reverse_rotate(a);
    }
    push(a, b); // pb
}

static void sort_3(t_stack* a)
{
    // Let's Hard code
    if (a->size != 3)
        return;
    int first = a->top->value;
    int second = a->top->next->value;
    int third = a->top->next->next->value;

    if (third > first && first > second) {
        swap(a);
    } else if (first > second && second > third) {
        swap(a);
        reverse_rotate(a);
    } else if (first > third && third > second) {
        rotate(a);
    } else if (second > third && third > first) {
        swap(a);
        rotate(a);
    } else if (second > first && first > third) {
        reverse_rotate(a);
    }
}

static void sort_4(t_stack* a, t_stack* b)
{
    if (a->size != 4) {
        return;
    }
    push_min_to_b(a, b); // 将最小值移到 b
    sort_3(a);
    push(b, a); // (pa)
}

static void sort_5(t_stack* a, t_stack* b)
{
    if (a->size != 5)
        return;

    // 1) 把最小的两个数依次推到 b
    push_min_to_b(a, b);
    push_min_to_b(a, b);

    // 2) 对剩下的 3 个数排序
    sort_3(a);

    // 3) 将 b 中的两个数按从大到小的顺序排序后依次推回 a
    if (b->top->value < b->top->next->value)
        swap(b);
    push(b, a); // pa
    push(b, a); // pa
}

void sort_stack(t_stack* a, t_stack* b)
{
    if (is_sorted(a)) {
        return;
    }
    switch (a->size) {
    case 2:
        if (a->top->value > a->top->next->value) {
            swap(a);
        }
        break;
    case 3:
        sort_3(a);
        break;
    case 4:
        sort_4(a, b);
        break;
    case 5:
        sort_5(a, b);
        break;
    default:
        radix_sort(a, b);
        break;
    }
}