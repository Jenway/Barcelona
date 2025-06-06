#if !defined(FDF_H)
#define FDF_H

#include <ft_printf.h>
#include <get_next_line.h>
#include <libft.h>
#include <mlx.h>
#include <unistd.h> // close, read, write
#include <fcntl.h>  // open

// 窗口默认尺寸
# define WIN_WIDTH  1280
# define WIN_HEIGHT  800

// 缩放系数，投影时 Z 轴缩放
#define SCALE_Z 0.1

// Linux (X11) 下常见的按键 Keycode
# define KEY_ESC      65307
# define KEY_LEFT     65361
# define KEY_RIGHT    65363
# define KEY_DOWN     65364
# define KEY_UP       65362
# define KEY_PLUS     61    // “=+” 键在不按 Shift 时返回 61，按 Shift 再取字符时是 '+'
# define KEY_MINUS    45    // “-” 键

# define KEY_W        119   // 'w'
# define KEY_S        115   // 's'
# define KEY_A        97    // 'a'
# define KEY_D        100   // 'd'
# define KEY_Q        113   // 'q'
# define KEY_E        101   // 'e'

// 原始网格中每个点
typedef struct s_point
{
	int x;
	int y;
	int z;
}               t_point;

// 加载后的地图
typedef struct s_map
{
	int     width;   // 列数
	int     height;  // 行数
	t_point **points;// 二维数组 points[y][x] 包含 {x, y, z}
}               t_map;

// 摄像机／视角参数
typedef struct s_camera
{
	double  x_ang;    // 绕 X 轴旋转（弧度）
	double  y_ang;    // 绕 Y 轴旋转
	double  z_ang;    // 绕 Z 轴旋转
	int     x_offset; // 在屏幕上平移 (px)
	int     y_offset; // 在屏幕上平移 (px)
	double  zoom;     // 缩放系数
}               t_camera;

// 图像缓冲及 MLX 环境
typedef struct s_data
{
	void    *mlx;
	void    *win;
	void    *img;
	char    *addr;
	int     bpp;
	int     line_len;
	int     endian;
	int     width;   // 图像宽度 = 窗口宽度
	int     height;  // 图像高度 = 窗口高度
}               t_data;

// 整个程序环境
typedef struct s_fdf
{
	t_map      *map;
	t_camera   *camera;
	t_data     *data;
}               t_fdf;

/*
 * map_loader.c
 */
t_list  *read_lines(const char *filename);
t_map   *parse_map(t_list *lines);
void    free_map(t_map *map);

/*
 * project_utils.c
 */
t_point project_point(t_point src, t_camera *cam);

/*
 * draw_utils.c
 */
// void    draw_line(t_data *data, t_point p0, t_point p1, int color);

/**
 *  draw_line_gradient
 *    @data: 图像缓冲
 *    @p0, @p1: 两个投影后的二维点
 *    @c0, @c1: 两点对应的 0xRRGGBB 颜色（分别由 z 决定）
 *
 *  函数内部会将线段从 p0→p1 平均分成若干步，按步数比例 t 逐像素插值 c0→c1。
 */
void    draw_line_gradient(t_data *data, t_point p0, t_point p1, int c0, int c1);

/*
 * render.c
 */
void    render_map(t_fdf *env);

/*
 * controls.c
 */
int     key_hook(int keycode, void *param);
void    install_hooks(t_fdf *env);


#endif // FDF_H
