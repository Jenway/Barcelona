#include "Cat.hpp"

Cat::Cat() {
    type = "Cat";
    std::cout << "Cat constructed\n";
}

Cat::~Cat() {
    std::cout << "Cat destructed\n";
}

void Cat::makeSound() const {
    std::cout << "Meow! 🐱\n";
}

