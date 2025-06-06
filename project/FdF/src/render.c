// render.c

#include "../FdF.h"
#include <math.h> // fmax, fmin

/*
 * 先遍历 map，计算 zmin 与 zmax
 */
static void compute_zmin_zmax(t_map* map, int* zmin, int* zmax)
{
    *zmin = map->points[0][0].z;
    *zmax = map->points[0][0].z;
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            int z = map->points[y][x].z;
            if (z < *zmin)
                *zmin = z;
            if (z > *zmax)
                *zmax = z;
        }
    }
}

#define LERP(a, b, t) \
    ((int)((a) + ((b) - (a)) * (t)))

// 将 z 映射到 [0,1] 范围，并处理边界
static double normalize_t(int z, int range_min, int range_max)
{
    if (range_max - range_min < 1e-6)
        return 0.0;
    double t = (double)(z - range_min) / (range_max - range_min);
    return fmax(0.0, fmin(1.0, t));
}

// 把 r、g、b 三个 0~255 再合成 0xRRGGBB
#define COMPOSE_RGB(r, g, b) \
    ((((r) & 0xFF) << 16) | (((g) & 0xFF) << 8) | (((b) & 0xFF) << 0))

/*
 * 根据 z ∈ [zmin, zmax] 线性插值到一个 0xRRGGBB 颜色：
 *   z = zmin    → 纯 Blue  (0x0000FF)
 *   z = (zmin+zmax)/2 → Pure Green (0x00FF00)
 *   z = zmax    → Pure Red   (0xFF0000)
 */
static int get_color_by_z(int z, int zmin, int zmax)
{
    double mid = ((double)zmin + zmax) * 0.5;
    double t;

    if (z <= mid) {
        t = normalize_t(z, zmin, mid); // t ∈ [0,1]
        int G = LERP(0, 255, t); // 0 → 255
        int B = LERP(255, 0, t); // 255 → 0
        return COMPOSE_RGB(0, G, B); // 0x00GGBB
    } else {
        t = normalize_t(z, mid, zmax); // t ∈ [0,1]
        int R = LERP(0, 255, t); // 0 → 255
        int G = LERP(255, 0, t); // 255 → 0
        return COMPOSE_RGB(R, G, 0); // 0xRRGG00
    }
}

void render_map(t_fdf* env)
{
    t_map* map = env->map;
    t_data* data = env->data;
    t_camera* cam = env->camera;
    t_point a3d, b3d;
    t_point a2d, b2d;
    int zmin, zmax;

    // 1) 先用 0 清空整张图像（黑色背景）
    ft_bzero(data->addr, data->height * data->line_len);

    // 2) 计算 zmin, zmax
    compute_zmin_zmax(map, &zmin, &zmax);

    // 3) 遍历每个点，按“右→下”方向连线
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            a3d = map->points[y][x];
            a2d = project_point(a3d, cam);

            // (1) 向右连一条线
            if (x + 1 < map->width) {
                b3d = map->points[y][x + 1];
                b2d = project_point(b3d, cam);
                int c0 = get_color_by_z(a3d.z, zmin, zmax);
                int c1 = get_color_by_z(b3d.z, zmin, zmax);
                draw_line_gradient(data, a2d, b2d, c0, c1);
            }
            // (2) 向下连一条线
            if (y + 1 < map->height) {
                b3d = map->points[y + 1][x];
                b2d = project_point(b3d, cam);
                int c0 = get_color_by_z(a3d.z, zmin, zmax);
                int c1 = get_color_by_z(b3d.z, zmin, zmax);
                draw_line_gradient(data, a2d, b2d, c0, c1);
            }
        }
    }

    // 4) 把整张图贴到窗口里
    mlx_put_image_to_window(data->mlx, data->win, data->img, 0, 0);
}