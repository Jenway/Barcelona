#include "Zombie.hpp"
#include <iostream>

Zombie::Zombie() {
    std::cout << "Zombie created.\n";
}

Zombie::~Zombie() {
    std::cout << "Zombie " << _name << " is destroyed.\n";
}

void Zombie::setName(std::string name) {
    _name = name;
}

void Zombie::announce() {
    std::cout << _name << ": BraiiiiiiinnnzzzZ...\n";
}
