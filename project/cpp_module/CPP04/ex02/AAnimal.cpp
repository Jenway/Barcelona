#include "AAnimal.hpp"

AAnimal::AAnimal() : type("AAnimal") {
    std::cout << "AAnimal constructed\n";
}

AAnimal::AAnimal(const AAnimal& other) : type(other.type) {
    std::cout << "AAnimal copy constructed\n";
}

AAnimal& AAnimal::operator=(const AAnimal& other) {
    std::cout << "AAnimal assigned\n";
    if (this != &other)
        type = other.type;
    return *this;
}

AAnimal::~AAnimal() {
    std::cout << "AAnimal destructed\n";
}

std::string AAnimal::getType() const {
    return type;
}

