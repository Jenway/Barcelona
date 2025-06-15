#include "Point.hpp"

static Fixed area(const Point& a, const Point& b, const Point& c) {
    return ((a.getX() * (b.getY() - c.getY())) +
            (b.getX() * (c.getY() - a.getY())) +
            (c.getX() * (a.getY() - b.getY()))) / 2;
}

bool bsp(Point const& a, Point const& b , Point const& c, Point const& point) {
    Fixed total = area(a, b, c);
    Fixed area1 = area(point, b, c);
    Fixed area2 = area(a, point, c);
    Fixed area3 = area(a, b, point);

    if (area1 == 0 || area2 == 0 || area3 == 0)
        return false;

    Fixed sum = area1 + area2 + area3;

    // 比较浮点值时最好使用 epsilon 判断，但 Fixed 类精度足够可以直接比较
    return sum == total;
}
