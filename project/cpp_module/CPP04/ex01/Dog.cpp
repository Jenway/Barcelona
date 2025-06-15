#include "Dog.hpp"

Dog::Dog() : brain(new Brain()) {
    type = "Dog";
    std::cout << "Dog constructed\n";
}

Dog::Dog(const Dog& other) : Animal(other) {
    std::cout << "Dog copy constructed\n";
    brain = new Brain(*other.brain);
}

Dog& Dog::operator=(const Dog& other) {
    std::cout << "Dog assigned\n";
    if (this != &other) {
        Animal::operator=(other);
        *brain = *other.brain;
    }
    return *this;
}

Dog::~Dog() {
    delete brain;
    std::cout << "Dog destructed\n";
}

void Dog::makeSound() const {
    std::cout << "Woof! 🐶\n";
}

Brain* Dog::getBrain() const {
    return brain;
}

