#ifndef POINT_HPP
#define POINT_HPP

#include "Fixed.hpp"

class Point {
private:
    const Fixed _x;
    const Fixed _y;

public:
    Point();
    Point(const float x, const float y);
    Point(const Point& other);
    ~Point();

    Point& operator=(const Point& other); // 复制赋值虽无意义，但仍需定义以符号规则

    Fixed getX() const;
    Fixed getY() const;
};

#endif
