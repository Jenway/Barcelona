#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <cstddef>
#include <stdexcept>

template <typename T>
class Array {
public:
    Array();                              // Default constructor
    Array(unsigned int n);               // Constructor with size
    Array(const Array& other);           // Copy constructor
    Array& operator=(const Array& other); // Assignment operator
    ~Array();                             // Destructor

    T& operator[](size_t index);             // Subscript operator
    const T& operator[](size_t index) const; // Const version

    size_t size() const; // Returns the size of the array

private:
    T* _data;
    size_t _size;
};

#include "Array.tpp"

#endif

