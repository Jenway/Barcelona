// controls.c

#include "../FdF.h"

static void adjust_camera(int keycode, t_camera* cam)
{
    if (keycode == KEY_LEFT)
        cam->y_ang -= 0.1;
    else if (keycode == KEY_RIGHT)
        cam->y_ang += 0.1;
    else if (keycode == KEY_UP)
        cam->x_ang -= 0.1;
    else if (keycode == KEY_DOWN)
        cam->x_ang += 0.1;
    else if (keycode == KEY_Q) // Z 轴正方向旋转
        cam->z_ang -= 0.1;
    else if (keycode == KEY_E) // Z 轴负方向旋转
        cam->z_ang += 0.1;
    else if (keycode == KEY_PLUS) // 放大
        cam->zoom += 1.0;
    else if (keycode == KEY_MINUS) // 缩小
        cam->zoom -= 1.0;
    else if (keycode == KEY_A) // 左移
        cam->x_offset -= 20;
    else if (keycode == KEY_D) // 右移
        cam->x_offset += 20;
    else if (keycode == KEY_W) // 上移
        cam->y_offset -= 20;
    else if (keycode == KEY_S) // 下移
        cam->y_offset += 20;

    // 限制 zoom 最小值不小于 1
    if (cam->zoom < 1.0)
        cam->zoom = 1.0;
}

// 键盘回调，返回 0 以继续事件循环
int key_hook(int keycode, void* param)
{
    t_fdf* env = (t_fdf*)param;

    if (keycode == KEY_ESC) {
        // 退出程序前释放资源
        ft_lstclear((t_list**)&env->map->points, free);
        free_map(env->map);
        mlx_destroy_image(env->data->mlx, env->data->img);
        mlx_destroy_window(env->data->mlx, env->data->win);
        exit(0);
    }
    adjust_camera(keycode, env->camera);
    render_map(env);
    return (0);
}

// 安装键盘事件钩子
void install_hooks(t_fdf* env)
{
    mlx_key_hook(env->data->win, key_hook, env);
}
