#include <GL/glut.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

// 定义旋转角度
float angleX = 0.0f;
float angleY = 0.0f;
float angleZ = 0.0f; // 新增：绕 z 轴的旋转角度

// 定义棋盘的尺寸
const int rows = 11;
const int cols = 19;

// 定义棋盘高度数据
std::vector<std::vector<int>> heightData(rows, std::vector<int>(cols, 0));

// 从文件读取高度数据
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
            iss >> heightData[i][j];
        }
    }
}

// 根据高度设置颜色
void setColorBasedOnHeight(int height)
{
    float color = height / 10.0f; // 将高度值映射到 [0, 1] 范围
    float r = 1.0f - 0.5f * color; // 从1到0.5
    float g = 1.0f - color; // 从1到0
    float b = 1.0f - 0.5f * color; // 从1到0.5
    glColor3f(r, g, b); // 颜色从白色到紫色渐变
}

// 绘制点，考虑高度
void drawPoint(float x, float y, float z)
{
    setColorBasedOnHeight(z * 10); // z 是高度除以 10 后的值，恢复原始高度
    glVertex3f(x, y, -z);
}

void drawLine(float x1, float y1, float z1, float x2, float y2, float z2)
{
    glBegin(GL_LINES);
    drawPoint(x1, y1, z1);
    drawPoint(x2, y2, z2);
    glEnd();
}
// 绘制棋盘，只显示点和线
void drawHeightMap()
{
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (j < cols - 1) {
                float x1 = j - cols / 2;
                float y1 = i - rows / 2;
                float z1 = heightData[i][j] / 10.0f;

                float x2 = (j + 1) - cols / 2;
                float y2 = i - rows / 2;
                float z2 = heightData[i][j + 1] / 10.0f;

                drawLine(x1, y1, z1, x2, y2, z2);
            }

            if (i < rows - 1) {
                float x1 = j - cols / 2;
                float y1 = i - rows / 2;
                float z1 = heightData[i][j] / 10.0f;

                float x3 = j - cols / 2;
                float y3 = (i + 1) - rows / 2;
                float z3 = heightData[i + 1][j] / 10.0f;

                drawLine(x1, y1, z1, x3, y3, z3);
            }
        }
    }
}

// 显示回调函数
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -30.0f);
    glRotatef(angleX, 1.0f, 0.0f, 0.0f);
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);
    glRotatef(angleZ, 0.0f, 0.0f, 1.0f); // 绕 z 轴旋转
    drawHeightMap();
    glutSwapBuffers();
}

// 键盘回调函数
void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'w':
        angleX -= 5.0f;
        break;
    case 's':
        angleX += 5.0f;
        break;
    case 'a':
        angleY -= 5.0f;
        break;
    case 'd':
        angleY += 5.0f;
        break;
    case 'q':
        angleZ -= 5.0f;
        break;
    case 'e':
        angleZ += 5.0f;
        break;
    case 27:
        exit(0);
        break; // ESC 键退出
    }
    glutPostRedisplay();
}

// 初始化 OpenGL 设置
void init()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    gluPerspective(45.0, 1.0, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

// 主函数
int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "用法: " << argv[0] << " <文件路径>" << std::endl;
        return 1;
    }

    readHeightData(argv[1]);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("3D Height Map Viewer");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
