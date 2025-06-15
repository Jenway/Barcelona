#include "Cat.hpp"

Cat::Cat() : brain(new Brain()) {
    type = "Cat";
    std::cout << "Cat constructed\n";
}

Cat::Cat(const Cat& other) : AAnimal(other) {
    std::cout << "Cat copy constructed\n";
    brain = new Brain(*other.brain);
}

Cat& Cat::operator=(const Cat& other) {
    std::cout << "Cat assigned\n";
    if (this != &other) {
        AAnimal::operator=(other);
        *brain = *other.brain;
    }
    return *this;
}

Cat::~Cat() {
    delete brain;
    std::cout << "Cat destructed\n";
}

void Cat::makeSound() const {
    std::cout << "Meow! 🐱\n";
}

Brain* Cat::getBrain() const {
    return brain;
}

