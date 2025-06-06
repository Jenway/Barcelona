// draw_utils.c
#include "../FdF.h"

#include <math.h>
#include <stdlib.h>

/*
 * 在 data 缓冲区里写一个像素，越界时直接 return
 */
static void my_mlx_pixel_put(t_data* data, int x, int y, int color)
{
    char* dst;

    if (x < 0 || x >= data->width || y < 0 || y >= data->height)
        return;
    dst = data->addr + (y * data->line_len + x * (data->bpp / 8));
    *(unsigned int*)dst = (unsigned int)color;
}

// 从 0xRRGGBB 中拆出 R、G、B 三个 0~255 的分量
#define EXTRACT_RGB(color, r, g, b)   \
    do {                              \
        (r) = ((color) >> 16) & 0xFF; \
        (g) = ((color) >> 8) & 0xFF;  \
        (b) = ((color) >> 0) & 0xFF;  \
    } while (0)

// 把 r、g、b 三个 0~255 再合成 0xRRGGBB
#define COMPOSE_RGB(r, g, b) \
    ((((r) & 0xFF) << 16) | (((g) & 0xFF) << 8) | (((b) & 0xFF) << 0))

/**
 * @brief 线性插值两个颜色（RGB 空间）
 * @param c0 起始颜色 (0xRRGGBB)
 * @param c1 结束颜色 (0xRRGGBB)
 * @param t  插值比例 [0.0, 1.0]
 * @return 插值后的颜色 (0xRRGGBB)
 */
static int lerp_color(int c0, int c1, double t)
{
    int r0, g0, b0, r1, g1, b1;
    EXTRACT_RGB(c0, r0, g0, b0);
    EXTRACT_RGB(c1, r1, g1, b1);

    int r = (int)(r0 + (r1 - r0) * t);
    int g = (int)(g0 + (g1 - g0) * t);
    int b = (int)(b0 + (b1 - b0) * t);

    return COMPOSE_RGB(r, g, b);
}

/*
 * draw_line_gradient：
 *   使用标准的 Bresenham 算法 (err = dx + dy, dy 为负) 画线，
 *   并在每一步根据“当前步数 i / 总步数 steps”在 c0→c1 之间线性插值出一个颜色，
 */
void draw_line_gradient(t_data* data, t_point p0, t_point p1, int c0, int c1)
{
    int dx = abs(p1.x - p0.x);
    int dy = -abs(p1.y - p0.y);
    int sx = (p0.x < p1.x) ? 1 : -1;
    int sy = (p0.y < p1.y) ? 1 : -1;
    int err = dx + dy;
    int steps = dx > -dy ? dx : -dy;
    if (steps == 0)
        steps = 1;

    for (int i = 0;; i++) {
        double t = (double)i / (double)steps;
        t = fmax(0.0, fmin(1.0, t)); // 确保 t ∈ [0,1]
        int color = lerp_color(c0, c1, t);
        my_mlx_pixel_put(data, p0.x, p0.y, color);

        if (p0.x == p1.x && p0.y == p1.y)
            break;

        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            p0.x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            p0.y += sy;
        }
    }
}
