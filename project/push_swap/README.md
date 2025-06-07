## push_swap

只用两个栈（其实是双链表）以及给定若干操作来实现排序。


> [!NOTE]
> 参考 <https://stackoverflow.com/a/75115367/22288601>
> 
> The term stack is incorrectly used to describe the containers a and b (possibly a French to English translation issue), as swap and rotate are not native stack operations. A common implementation uses a circular doubly-linked list.

### 硬编码

对于较小的数字，比如2、3、4、5，很容易就能穷举。

> [Push_Swap: The least amount of moves with two stacks](https://medium.com/@jamierobertdawson/push-swap-the-least-amount-of-moves-with-two-stacks-d1e76a71789a)

- 2：如果无序，交换即可
- 3：穷举 5 种可能即可
- 4：取最小到 b 栈，然后对于 a 栈排序后，再把最小值加回来
- 5：类似 4，不过取最小两个数到 b 栈

### Radix Sort

其实还有种分 chunk 的思路，类似于插入排序的某种变式？

那么基数排序默认不能处理负数，所以要预处理（平移负数），然后确定要用的最大位数。

然后就是**按位排序**（LSD 优先）了:

- 从最低位（`i=0`）到最高位（`i=max_bits-1`）遍历：
    - 遍历栈 `a` 的当前所有元素：
    - 取栈顶元素，计算平移值 `key = value + shift`。
    - **检查第 `i` 位**:
        - 若为 `1`：执行 `rotate(a)`（`ra`），将元素移至栈底。
        - 若为 `0`：执行 `push(a, b)`（`pb`），移入栈 `b`。
    - 将栈 `b` 所有元素压回栈 `a`（`push(b, a)`/`pa`）。

- **空间复杂度**: O(1)（用两个栈）。
- **时间复杂度**: O(k·n)，`k=max_bits`（约 32/64 位），`n` 为元素数量。
- **稳定性**: 非稳定排序（相同位值的元素相对顺序可能改变）。
- **负数支持**: 通过平移转为非负处理，排序后自动还原（因 `shift` 仅用于计算）。

### 速度要求

准确的说是操作次数（instructions）要求

One web site lists how push swap is scored.

```
required: sort   3 numbers with <=     3 operations
required: sort   5 numbers with <=    12 operations
scored:   sort 100 numbers with <=   700 operations   max score
                                     900 operations
                                    1100 operations
                                    1300 operations
                                    1500 operations   min score
scored:   sort 500 numbers with <=  5500 operations   max score
                                    7000 operations
                                    8500 operations
                                   10000 operations
                                   11500 operations   min score
```

### Visualizer

一定要试一试 <https://github.com/o-reo/push_swap_visualizer> 很不错。

![push_swap_visualizer](visualizer.png)
