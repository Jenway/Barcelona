#ifndef SERIALIZER_HPP
#define SERIALIZER_HPP

#include "Data.hpp"

class Serializer {
private:
    Serializer();
    ~Serializer();
    Serializer(const Serializer&);
    Serializer& operator=(const Serializer&);

public:
    static unsigned long serialize(Data* ptr);          // use unsigned long instead of uintptr_t
    static Data* deserialize(unsigned long raw);
};

#endif

