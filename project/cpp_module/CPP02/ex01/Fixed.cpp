#include "Fixed.hpp"
#include <cmath>  // roundf

Fixed::Fixed() : fixed_point_value(0) {
    std::cout << "Default constructor called\n";
}

Fixed::Fixed(const int int_val) {
    std::cout << "Int constructor called\n";
    fixed_point_value = int_val << fractional_bits; // 左移8位相当于乘以256
}

Fixed::Fixed(const float float_val) {
    std::cout << "Float constructor called\n";
    fixed_point_value = static_cast<int>(roundf(float_val * (1 << fractional_bits)));
}

Fixed::Fixed(const Fixed& other) {
    std::cout << "Copy constructor called\n";
    fixed_point_value = other.fixed_point_value;
}

Fixed& Fixed::operator=(const Fixed& other) {
    std::cout << "Copy assignment operator called\n";
    if (this != &other) {
        fixed_point_value = other.fixed_point_value;
    }
    return *this;
}

Fixed::~Fixed() {
    std::cout << "Destructor called\n";
}

int Fixed::getRawBits(void) const {
    std::cout << "getRawBits member function called\n";
    return fixed_point_value;
}

void Fixed::setRawBits(int const raw) {
    fixed_point_value = raw;
}

float Fixed::toFloat(void) const {
    return static_cast<float>(fixed_point_value) / (1 << fractional_bits);
}

int Fixed::toInt(void) const {
    return fixed_point_value >> fractional_bits;
}

std::ostream& operator<<(std::ostream& os, const Fixed& fixed) {
    os << fixed.toFloat();
    return os;
}
