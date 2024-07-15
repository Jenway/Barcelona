#include "../libft.h"

//  ft_memmove()
// The  memmove()  function  copies n bytes from memory area src to memory area dest.  The
//    memory areas may overlap: copying takes place as though the  bytes  in  src  are  first
//    copied into a temporary array that does not overlap src or dest, and the bytes are then
//    copied from the temporary array to dest.

// 自定义的内存移动函数，功能类似于标准库中的memmove
void* ft_memmove(void* dest, const void* src, size_t n)
{
    // 定义目标和源的指针，转换为unsigned char*以逐字节操作内存
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;

    // 如果目标和源都为NULL，则直接返回NULL
    if (!dest && !src) {
        return (NULL);
    }

    // 如果目标地址小于源地址，从前向后复制，避免覆盖
    if (d < s) {
        while (n--) {
            *d++ = *s++; // 逐字节复制
        }
    } else {
        // 如果目标地址大于源地址，从后向前复制，避免覆盖
        d += n;
        s += n;
        while (n--) {
            *--d = *--s; // 逐字节复制
        }
    }

    // 返回目标内存的指针
    return (dest);
}