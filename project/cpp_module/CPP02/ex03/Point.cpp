#include "Point.hpp"

Point::Point(void) : _x(Fixed(0)), _y(Fixed(0)) { }

Point::Point(const float x, const float y) : _x(Fixed(x)), _y(Fixed(y)) { }

// overloaded =
Point::Point(const Point& other) { *this = other; }

Point::~Point() {}

Point& Point::operator=(const Point& other) {
    // 无法真正赋值 _x 和 _y（它们是 const），但要定义这个函数
    // 所以只是返回 *this 并输出警告（可选）
    (void)other;
    return *this;
}

Fixed Point::getX() const {
    return _x;
}

Fixed Point::getY() const {
    return _y;
}
