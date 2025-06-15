#ifndef FIXED_HPP
#define FIXED_HPP

class Fixed {
private:
    int _rawBits;                             // 用来表示 fixed-point 数值的整数
    static const int _fractionalBits = 8;     // 固定为 8 位小数

public:
    Fixed();                                  // 默认构造函数
    Fixed(const Fixed& other);                // 拷贝构造函数
    Fixed& operator=(const Fixed& other);     // 拷贝赋值运算符
    ~Fixed();                                 // 析构函数

    int getRawBits() const;                   // 获取原始值
    void setRawBits(int const raw);           // 设置原始值
};

#endif