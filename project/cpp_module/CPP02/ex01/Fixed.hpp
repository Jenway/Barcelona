#ifndef FIXED_HPP
#define FIXED_HPP

#include <iostream>

class Fixed {
private:
    int fixed_point_value;
    static const int fractional_bits = 8;

public:
    Fixed();                     // 默认构造
    Fixed(const int int_val);    // 整数构造
    Fixed(const float float_val);// 浮点构造
    Fixed(const Fixed& other);   // 复制构造
    Fixed& operator=(const Fixed& other); // 赋值操作符
    ~Fixed();

    int getRawBits(void) const;
    void setRawBits(int const raw);

    float toFloat(void) const;
    int toInt(void) const;
};

// 流输出运算符重载，声明为友元或普通函数
std::ostream& operator<<(std::ostream& os, const Fixed& fixed);

#endif
