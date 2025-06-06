// project_utils.c

#include "../FdF.h"
#include <math.h> // sin, cos

/*
 * 将原始三维点 src (x, y, z) 按照 camera 参数做三轴旋转，
 */
t_point project_point(t_point src, t_camera* cam)
{
    t_point res;
    double x = src.x;
    double y = src.y;
    double z = src.z;
    double y1, z1, x2, z2, x3, y3;

    // 绕 X 轴
    y1 = y * cos(cam->x_ang) - z * sin(cam->x_ang);
    z1 = y * sin(cam->x_ang) + z * cos(cam->x_ang);

    // 绕 Y 轴
    x2 = x * cos(cam->y_ang) + z1 * sin(cam->y_ang);
    z2 = -x * sin(cam->y_ang) + z1 * cos(cam->y_ang);

    // 绕 Z 轴
    x3 = x2 * cos(cam->z_ang) - y1 * sin(cam->z_ang);
    y3 = x2 * sin(cam->z_ang) + y1 * cos(cam->z_ang);

    // 正交投影
    res.x = (int)((x3 - y3) * cos(M_PI / 6) * cam->zoom + cam->x_offset);
    res.y = (int)((x3 + y3) * sin(M_PI / 6) * cam->zoom - src.z * cam->zoom * SCALE_Z + cam->y_offset);
    res.z = 0; // 投影后不再需要 z 分量
    return (res);
}