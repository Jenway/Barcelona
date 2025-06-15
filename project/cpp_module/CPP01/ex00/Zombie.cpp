#include "Zombie.hpp"
#include <iostream>

Zombie::Zombie(std::string name) : _name(name) {
    std::cout << "Zombie " << _name << " is created.\n";
}

Zombie::~Zombie() {
    std::cout << "Zombie " << _name << " is destroyed.\n";
}

void Zombie::announce() {
    std::cout << _name << ": BraiiiiiiinnnzzzZ...\n";
}
