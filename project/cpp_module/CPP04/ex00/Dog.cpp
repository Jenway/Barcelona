#include "Dog.hpp"

Dog::Dog() {
    type = "Dog";
    std::cout << "Dog constructed\n";
}

Dog::~Dog() {
    std::cout << "Dog destructed\n";
}

void Dog::makeSound() const {
    std::cout << "Woof! 🐶\n";
}

