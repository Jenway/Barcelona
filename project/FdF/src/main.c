// main.c

#include "../FdF.h"

/*
 * 简单的打印错误信息并退出
 */
#define THROW(msg)                        \
    {                                     \
        ft_printf(" !! KO !! %s\n", msg); \
        exit(1);                          \
    }
#define LOG(msg) ft_printf(" >> %s\n", msg)

t_camera* create_camera(void)
{
    t_camera* cam = (t_camera*)malloc(sizeof(t_camera));
    if (!cam)
        return NULL;
    cam->x_ang = 0.0;
    cam->y_ang = 0.0;
    cam->z_ang = 0.0;
    cam->zoom = 20.0;
    cam->x_offset = WIN_WIDTH / 2;
    cam->y_offset = WIN_HEIGHT / 2;
    return cam;
}

t_data* init_mlx(void)
{
    t_data* data = (t_data*)malloc(sizeof(t_data));
    if (!data) {
        LOG("malloc data failed");
        return NULL;
    }

    data->mlx = mlx_init();
    if (!data->mlx) {
        LOG("mlx_init failed");
        free(data);
        return NULL;
    }

    data->win = mlx_new_window(data->mlx, WIN_WIDTH, WIN_HEIGHT, "FDF");
    if (!data->win) {
        LOG("mlx_new_window failed");
        free(data->mlx);
        free(data);
        return NULL;
    }

    data->img = mlx_new_image(data->mlx, WIN_WIDTH, WIN_HEIGHT);
    if (!data->img) {
        LOG("mlx_new_image failed");
        mlx_destroy_window(data->mlx, data->win);
        free(data->mlx);
        free(data);
        return NULL;
    }

    data->addr = mlx_get_data_addr(data->img, &data->bpp,
        &data->line_len, &data->endian);
    data->width = WIN_WIDTH;
    data->height = WIN_HEIGHT;
    return data;
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        ft_printf("Usage: %s <map_file.fdf>\n", argv[0]);
        return (EXIT_FAILURE);
    }
    t_list* lines;
    t_map* map;
    t_camera* cam;
    t_data* data;

    // 1. 读取文件到 t_list 链表
    if (!(lines = read_lines(argv[1]))) {
        ft_printf("Error: Could not read file '%s'\n", argv[1]);
        return (EXIT_FAILURE);
    }

    // 2. 解析成 t_map
    if (!(map = parse_map(lines))) {
        ft_lstclear(&lines, free);
        THROW("parse_map failed");
    }
    ft_lstclear(&lines, free);

    // 3. 分配并初始化摄像机参数
    if (!(cam = create_camera())) {
        free_map(map);
        THROW("create_camera failed");
    }

    // 4. 初始化 MLX，创建窗口并新建图像
    if (!(data = init_mlx())) {
        free_map(map);
        free(cam);
        THROW("init_mlx failed");
    }

    // 5. 把所有结构绑定到 env
    t_fdf env;
    env.map = map;
    env.camera = cam;
    env.data = data;

    // 6. 安装键盘回调（按键触发重绘）
    install_hooks(&env);

    // 7. 首次渲染
    render_map(&env);

    // 8. 进入事件循环
    mlx_loop(data->mlx);

    // 正常情况下不会执行到这里
    return (0);
}
