#include "Fixed.hpp"
#include <cmath>

Fixed::Fixed() : _fixedPointValue(0) {}

Fixed::Fixed(const int n) {
    _fixedPointValue = n << _fractionalBits;
}

Fixed::Fixed(const float f) {
    _fixedPointValue = static_cast<int>(roundf(f * (1 << _fractionalBits)));
}

Fixed::Fixed(const Fixed& other) {
    _fixedPointValue = other._fixedPointValue;
}

Fixed::~Fixed() {}

Fixed& Fixed::operator=(const Fixed& other) {
    if (this != &other)
        _fixedPointValue = other._fixedPointValue;
    return *this;
}

int Fixed::getRawBits(void) const {
    return _fixedPointValue;
}

void Fixed::setRawBits(int const raw) {
    _fixedPointValue = raw;
}

float Fixed::toFloat(void) const {
    return static_cast<float>(_fixedPointValue) / (1 << _fractionalBits);
}

int Fixed::toInt(void) const {
    return _fixedPointValue >> _fractionalBits;
}

// 比较运算符
bool Fixed::operator>(const Fixed& other) const {
    return _fixedPointValue > other._fixedPointValue;
}
bool Fixed::operator<(const Fixed& other) const {
    return _fixedPointValue < other._fixedPointValue;
}
bool Fixed::operator>=(const Fixed& other) const {
    return _fixedPointValue >= other._fixedPointValue;
}
bool Fixed::operator<=(const Fixed& other) const {
    return _fixedPointValue <= other._fixedPointValue;
}
bool Fixed::operator==(const Fixed& other) const {
    return _fixedPointValue == other._fixedPointValue;
}
bool Fixed::operator!=(const Fixed& other) const {
    return _fixedPointValue != other._fixedPointValue;
}

// 算术运算符
Fixed Fixed::operator+(const Fixed& other) const {
    Fixed result;
    result._fixedPointValue = this->_fixedPointValue + other._fixedPointValue;
    return result;
}

Fixed Fixed::operator-(const Fixed& other) const {
    Fixed result;
    result._fixedPointValue = this->_fixedPointValue - other._fixedPointValue;
    return result;
}

Fixed Fixed::operator*(const Fixed& other) const {
    Fixed result;
    // 乘法后需除以2^fractionalBits来还原精度
    long long temp = static_cast<long long>(this->_fixedPointValue) * other._fixedPointValue;
    result._fixedPointValue = static_cast<int>(temp >> _fractionalBits);
    return result;
}

Fixed Fixed::operator/(const Fixed& other) const {
    if (other._fixedPointValue == 0) {
        // 程序崩溃是允许的，下面为简单的保护
        std::cerr << "Error: Division by zero!" << std::endl;
        exit(1);
    }
    Fixed result;
    // 除法先放大再除，避免丢失精度
    long long temp = (static_cast<long long>(this->_fixedPointValue) << _fractionalBits) / other._fixedPointValue;
    result._fixedPointValue = static_cast<int>(temp);
    return result;
}

// 自增自减运算符
Fixed& Fixed::operator++() {
    _fixedPointValue += 1;  // 增加最小单位
    return *this;
}

Fixed Fixed::operator++(int) {
    Fixed temp(*this);
    ++(*this);
    return temp;
}

Fixed& Fixed::operator--() {
    _fixedPointValue -= 1;  // 减少最小单位
    return *this;
}

Fixed Fixed::operator--(int) {
    Fixed temp(*this);
    --(*this);
    return temp;
}

// min/max 静态函数
Fixed& Fixed::min(Fixed& a, Fixed& b) {
    return (a < b) ? a : b;
}

const Fixed& Fixed::min(const Fixed& a, const Fixed& b) {
    return (a < b) ? a : b;
}

Fixed& Fixed::max(Fixed& a, Fixed& b) {
    return (a > b) ? a : b;
}

const Fixed& Fixed::max(const Fixed& a, const Fixed& b) {
    return (a > b) ? a : b;
}

// 流插入运算符
std::ostream& operator<<(std::ostream& os, const Fixed& fixed) {
    os << fixed.toFloat();
    return os;
}
