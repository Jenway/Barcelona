一个简易的 FdF（“Fil de Fer”）线框渲染程序，实现了在 Linux + MiniLibX 环境下对 `.fdf` 格式地图文件的可视化。

![效果图](FdF_42.png)

1. **读取与解析**：把 `.fdf` 文件按行读到内存，并解析成二维点阵 `t_map`。
2. **三轴旋转 + 投影**：针对每个三维点 `(x, y, z)`，先按摄像机参数（绕 X/Y/Z 三轴的旋转）进行旋转运算，再做二轴测投影，得到屏幕上的 `(x', y')`。
3. **Bresenham 画线 + 颜色渐变**：遍历点阵，把每个点与右侧、下方相邻点连成线。在画线时基于两端高度做线性颜色插值，实现“高度渐变”效果。
4. **事件交互**：注册键盘回调，使得按键触发后重新渲染。

## MiniLibX

参考 [MiniLibX | 42 Docs](https://harm-smits.github.io/42docs/libs/minilibx)

> For me, one of the most time consuming things is trying to find out how the god damn 42 libraries work.

我同意这句话

## 运行示例

```bash
# 编译
make

# 运行
./fdf test_maps/42.fdf
```

* 初次打开窗口会看到彩色渐变的线框地图。
* 按键操作：

  * `←` / `→`：绕 Y 轴旋转
  * `↑` / `↓`：绕 X 轴旋转
  * `Q` / `E`：绕 Z 轴旋转
  * `A` / `D`：水平平移
  * `W` / `S`：垂直平移
  * `+` / `-`：整体缩放
  * `Z` / `X`：浮雕高度调节（放大/缩小）
  * `ESC`：退出

## Bear

这个编译使用了外部模块，所以 `clangd` 又罢工了，可以使用 `Bear` 来生成 `compile_commands.json` 来帮助 LSP 工作

## 主要模块与功能

### 1. `map_loader.c`（读取与解析模块）

* `t_list *read_lines(const char *filename)`
  * 将文件写入链表
* `t_map *parse_map(t_list *lines)`
  * 解析链表为 map
* `void free_map(t_map *map)`
  * 释放资源

---

### 2. `project_utils.c`（三轴旋转 + 投影模块）

```c
t_point project_point(t_point src, t_camera* cam)
```

对 `src` 点进行旋转和投影

#### 三维旋转（欧拉角）

1. **X 轴旋转（横滚角 Roll）**：

  **公式**：
  ```math
  \begin{aligned}
  y_1 &= y \cos\theta_x - z \sin\theta_x \\
  z_1 &= y \sin\theta_x + z \cos\theta_x
  \end{aligned}
  ```
  **矩阵形式**：
  ```math
  \begin{bmatrix}
  1 & 0 & 0 \\
  0 & \cos\theta_x & -\sin\theta_x \\
  0 & \sin\theta_x & \cos\theta_x
  \end{bmatrix}
  ```
2. **Y 轴旋转（俯仰角 Pitch）**：

  **公式**：
  ```math
  \begin{aligned}
  x_2 &= x \cos\theta_y + z_1 \sin\theta_y \\
  z_2 &= -x \sin\theta_y + z_1 \cos\theta_y
  \end{aligned}
  ```
  **矩阵形式**：
  ```math
  \begin{bmatrix}
  \cos\theta_y & 0 & \sin\theta_y \\
  0 & 1 & 0 \\
  -\sin\theta_y & 0 & \cos\theta_y
  \end{bmatrix}
  ```
3. **Z 轴旋转（偏航角 Yaw）**：

  **公式**：
  ```math
  \begin{aligned}
  x_3 &= x_2 \cos\theta_z - y_1 \sin\theta_z \\
  y_3 &= x_2 \sin\theta_z + y_1 \cos\theta_z
  \end{aligned}
  ```
  **矩阵形式**：
  ```math
  \begin{bmatrix}
  \cos\theta_z & -\sin\theta_z & 0 \\
  \sin\theta_z & \cos\theta_z & 0 \\
  0 & 0 & 1
  \end{bmatrix}
  ```

#### 正交投影（二轴测投影，Dimetric Projection）

**投影公式**：

```math
\begin{aligned}
x_{投影} &= (x_3 - y_3) \cos\frac{\pi}{6} \cdot \text{缩放} + x_{偏移} \\
y_{投影} &= (x_3 + y_3) \sin\frac{\pi}{6} \cdot \text{缩放} - z \cdot \text{缩放} \cdot k_z + y_{偏移}
\end{aligned}
```

**矩阵形式**

$$
P_{dimetric} = 
\begin{bmatrix}
k \cos\theta & -k \cos\theta & 0 \\
k \sin\theta & k \sin\theta & -k \cdot k_z \\
0 & 0 & 0
\end{bmatrix}, \quad \text{其中 } \theta = \frac{\pi}{6}, \ k = \text{zoom}
$$

- 其中 `k_z`（即代码中的`SCALE_Z`）控制深度缩放效果
   - 标准正交投影直接丢弃 $z$ 坐标（$P_{ortho}$）。
   - 二轴测投影保留了 $z$ 坐标对 $y$ 方向的影响（通过 $k_z$ 实现伪深度效果）。

  ```c
    // 正交投影
    res.x = (int)((x3 - y3) * cos(M_PI / 6) * cam->zoom
            + cam->x_offset);
    res.y = (int)((x3 + y3) * sin(M_PI / 6) * cam->zoom
            - src.z * cam->zoom * SCALE_Z
            + cam->y_offset);
    res.z = 0; // 投影后不再需要 z 分量
  ```

#### tldr

1. **旋转顺序**：X → Y → Z（横滚→俯仰→偏航）
2. **投影类型**：固定30°角的二轴测正交投影（通过`π/6`实现）
3. **深度处理**：保留原始z坐标(`src.z`)用于模拟伪透视效果
4. **z 轴归零**：最终z坐标归零（`res.z=0`）表示转换为纯2D坐标

---

### 3. `draw_utils.c`（Bresenham 算法 + RGB 线性插值）

```c
void draw_line_gradient(t_data *data, t_point p0, t_point p1, int c0, int c1);
```

* `data`：包含 MiniLibX 的图像缓冲和参数（`addr, bpp, line_len, endian, width, height`）。
* `p0, p1`：投影后的两个二维端点 `(x, y)`。
* `c0, c1`：两端点对应的颜色（`0xRRGGBB`）。

#### Bresenham 算法：经典光栅化直线方法

Bresenham 算法所有运算均为整数加减法，无浮点操作,因此效率很高。


**1. 输入与初始化**

给定起点 $P_0 = (x_0, y_0)$ 和终点 $P_1 = (x_1, y_1)$，定义：

- 坐标差值：
  $$
  \Delta x = |x_1 - x_0|, \quad \Delta y = |y_1 - y_0|
  $$
- 步进方向：
  $$
  s_x = \begin{cases} 
  1 & \text{if } x_1 > x_0 \\
  -1 & \text{otherwise}
  \end{cases}, \quad
  s_y = \begin{cases} 
  1 & \text{if } y_1 > y_0 \\
  -1 & \text{otherwise}
  \end{cases}
  $$
- 初始误差项：
  $$
  e = \Delta x - \Delta y
  $$

**2. 迭代过程**

对于每一点 $(x, y)$，重复以下步骤直到 $(x, y) = (x_1, y_1)$：
1. **绘制当前点**：
   $$
   \text{draw\_pixel}(x, y)
   $$
2. **误差更新与步进**：
   - 计算误差的 2 倍：
     $$
     e_2 = 2 \cdot e
     $$
   - 若 $e_2 > -\Delta y$：
     $$
     e \leftarrow e - \Delta y, \quad x \leftarrow x + s_x
     $$
   - 若 $e_2 < \Delta x$：
     $$
     e \leftarrow e + \Delta x, \quad y \leftarrow y + s_y
     $$

**3. 终止条件**

当且仅当：
$$
x = x_1 \land y = y_1
$$

#### RGB 线性插值：实现起点到终点的颜色平滑过渡

根据当前位置在整条线段上的比例 `t`，把起点/终点的 RGB 通道做线性混合，从而实现“线段内的平滑渐变”效果。

**线性插值（Linear Interpolation, Lerp）** 是一种在两个已知值之间**按比例计算中间值**的方法。数学定义为：

$$
\text{lerp}(a, b, t) = a + (b - a) \cdot t \quad \text{其中} \ t \in [0, 1]
$$

- **物理意义**：当 $t$ 从 0 变化到 1 时，结果从 $a$ 平滑过渡到 $b$。


在**RGB 颜色空间中的线性插值**中，颜色被分解为 **红（R）、绿（G）、蓝（B）** 三个分量，分别独立插值：

若追求视觉效果，可扩展为 HSV 插值；

### 4. `render.c`（整体渲染逻辑）

  ```c
  void render_map(t_fdf *env);
  ```

  * `env->map`：包含二维点阵 `points[y][x]` 和 `width/height`。
  * `env->camera`：当前视角参数。
  * `env->data`：MiniLibX 图像缓冲。

**渲染流程**

1. 帧缓冲清零（黑色背景）
2. **高程-颜色映射**
  - **高程采样**	`compute_zmin_zmax()`	遍历所有顶点，记录最小/最大Z值
  - **颜色映射**	`get_color_by_z()`	分段线性插值：
    - 低区间：蓝→绿（B↓G↑）
    - 高区间：绿→红（G↓R↑）
3. 网格遍历渲染：
  - 水平方向连接 (x,y)-(x+1,y)
  - 垂直方向连接 (x,y)-(x,y+1)
4. 提交到显示系统

---

### 5. `controls.c`（键盘交互模块）

绑定键盘回调与设置循环。
