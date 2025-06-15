#include "WrongAnimal.hpp"

WrongAnimal::WrongAnimal() : type("WrongAnimal") {
    std::cout << "WrongAnimal constructed\n";
}

WrongAnimal::~WrongAnimal() {
    std::cout << "WrongAnimal destructed\n";
}

std::string WrongAnimal::getType() const {
    return type;
}

void WrongAnimal::makeSound() const {
    std::cout << "* wrong animal sound *\n";
}

