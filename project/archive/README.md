# Archive - OpenGL 线框查看器

一个基于 OpenGL 的 3D 线框地形可视化工具，用于查看和交互式操作高度图数据。这是一个独立的归档项目，演示了 OpenGL 图形编程和 3D 数据可视化的基本技术。

## 项目概述

这个 OpenGL 线框查看器能够：

- **读取高度数据文件**：解析包含高度信息的文本格式地图文件
- **3D 线框渲染**：将 2D 高度数据转换为 3D 线框模型
- **交互式控制**：支持实时的 3D 旋转和视角调整
- **流畅动画**：提供平滑的键盘交互响应

## 功能特性

### 1. 数据处理
- 支持文本格式的高度图文件
- 自动解析网格数据结构
- 灵活的数据尺寸适配

### 2. 3D 渲染
- OpenGL 线框模式渲染
- 深度缓冲支持，正确处理遮挡关系
- 透视投影，提供真实的 3D 视觉效果

### 3. 交互控制
| 按键 | 功能 |
|------|------|
| `W` | 绕 X 轴向上旋转 |
| `S` | 绕 X 轴向下旋转 |
| `A` | 绕 Y 轴向左旋转 |
| `D` | 绕 Y 轴向右旋转 |
| `Q` | 绕 Z 轴逆时针旋转 |
| `E` | 绕 Z 轴顺时针旋转 |
| `ESC` | 退出程序 |

## 编译与运行

### 依赖库
- **OpenGL**: 3D 图形渲染库
- **GLUT**: OpenGL 工具库（窗口管理和事件处理）
- **GLU**: OpenGL 实用库（高级几何函数）

### 编译命令

#### Linux/Unix
```bash
g++ -o wireframe_viewer wireframe_viewer_GL.cpp -lGL -lGLU -lglut
```

#### macOS
```bash
g++ -o wireframe_viewer wireframe_viewer_GL.cpp -framework OpenGL -framework GLUT
```

#### Windows (MinGW)
```bash
g++ -o wireframe_viewer.exe wireframe_viewer_GL.cpp -lopengl32 -lglu32 -lfreeglut
```

### 运行程序

```bash
./wireframe_viewer <高度图文件>
```

**示例**：
```bash
./wireframe_viewer heightmap.txt
```

## 数据文件格式

程序期望的输入文件格式为纯文本，包含由空格分隔的整数高度值：

```
0  0  1  2  3  4  5  4  3  2  1  0  0
0  1  2  3  4  5  6  5  4  3  2  1  0
1  2  3  4  5  6  7  6  5  4  3  2  1
2  3  4  5  6  7  8  7  6  5  4  3  2
3  4  5  6  7  8  9  8  7  6  5  4  3
4  5  6  7  8  9 10  9  8  7  6  5  4
5  6  7  8  9 10 11 10  9  8  7  6  5
4  5  6  7  8  9 10  9  8  7  6  5  4
3  4  5  6  7  8  9  8  7  6  5  4  3
2  3  4  5  6  7  8  7  6  5  4  3  2
1  2  3  4  5  6  7  6  5  4  3  2  1
```

## 核心实现

### 1. 数据读取与解析

```cpp
void readHeightData(const char* filename)
{
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        exit(1);
    }

    std::string line;
    for (int i = 0; i < rows && std::getline(file, line); ++i) {
        std::istringstream iss(line);
        for (int j = 0; j < cols; ++j) {
            int height;
            if (iss >> height) {
                heightData[i][j] = height;
            }
        }
    }
    file.close();
}
```

### 2. 3D 线框渲染

```cpp
void drawHeightMap()
{
    glColor3f(1.0f, 1.0f, 1.0f);  // 白色线条
    
    // 绘制水平线（行连接）
    for (int i = 0; i < rows; ++i) {
        glBegin(GL_LINE_STRIP);
        for (int j = 0; j < cols; ++j) {
            float x = j - cols / 2.0f;         // 中心化 X 坐标
            float y = heightData[i][j] * 0.2f; // 缩放高度
            float z = i - rows / 2.0f;         // 中心化 Z 坐标
            glVertex3f(x, y, z);
        }
        glEnd();
    }
    
    // 绘制垂直线（列连接）
    for (int j = 0; j < cols; ++j) {
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i < rows; ++i) {
            float x = j - cols / 2.0f;
            float y = heightData[i][j] * 0.2f;
            float z = i - rows / 2.0f;
            glVertex3f(x, y, z);
        }
        glEnd();
    }
}
```

### 3. 交互式旋转控制

```cpp
void keyboard(unsigned char key, int x, int y)
{
    const float rotationStep = 5.0f;
    
    switch (key) {
    case 'w': angleX -= rotationStep; break;  // 上
    case 's': angleX += rotationStep; break;  // 下
    case 'a': angleY -= rotationStep; break;  // 左
    case 'd': angleY += rotationStep; break;  // 右
    case 'q': angleZ -= rotationStep; break;  // 逆时针
    case 'e': angleZ += rotationStep; break;  // 顺时针
    case 27: exit(0); break;                  // ESC 退出
    }
    
    glutPostRedisplay();  // 请求重绘
}
```

### 4. OpenGL 渲染循环

```cpp
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    // 设置视点
    gluLookAt(0.0, 10.0, 20.0,    // 相机位置
              0.0, 0.0, 0.0,      // 注视点
              0.0, 1.0, 0.0);     // 上方向
    
    // 应用旋转变换
    glRotatef(angleX, 1.0f, 0.0f, 0.0f);  // 绕 X 轴
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);  // 绕 Y 轴  
    glRotatef(angleZ, 0.0f, 0.0f, 1.0f);  // 绕 Z 轴
    
    drawHeightMap();
    glutSwapBuffers();  // 双缓冲交换
}
```

## 技术特点

### 1. 高效渲染
- 使用 `GL_LINE_STRIP` 减少顶点数据传输
- 双缓冲机制确保流畅的动画效果
- 深度测试正确处理 3D 遮挡关系

### 2. 灵活的数据处理
- 运行时确定数据尺寸
- 自动坐标中心化
- 高度数据缩放以获得合适的视觉效果

### 3. 直观的用户交互
- 响应式键盘控制
- 平滑的旋转动画
- 实时视角调整

## 应用场景

1. **地形数据可视化**：查看 DEM（数字高程模型）数据
2. **数学函数图形**：可视化二元函数的图形
3. **科学数据分析**：展示网格化的科学数据
4. **教育演示**：3D 图形编程的教学示例
5. **原型开发**：3D 可视化应用的快速原型

## 扩展可能性

虽然这是一个归档项目，但可以考虑以下改进方向：

1. **颜色映射**：根据高度值应用不同颜色
2. **纹理支持**：添加表面纹理渲染
3. **光照模型**：实现更真实的光照效果
4. **鼠标交互**：支持鼠标拖拽旋转
5. **多种渲染模式**：线框、实体、点云等模式切换

## 学习价值

这个项目展示了：

- **OpenGL 基础**：基本的 3D 图形编程概念
- **数据可视化**：将数值数据转换为视觉表示
- **交互设计**：用户界面的响应式设计
- **几何变换**：3D 空间中的旋转和变换
- **文件 I/O**：结构化数据的读取和解析

---

这个线框查看器虽然功能相对简单，但它是学习 3D 图形编程和数据可视化的极佳起点，展示了如何用相对少量的代码实现功能完整的 3D 可视化工具。