#include "Ice.hpp"
#include "ICharacter.hpp"
#include <iostream>

Ice::Ice() : AMateria("ice") {
    std::cout << "Ice constructed\n";
}

Ice::Ice(const Ice& other) : AMateria(other) {
    std::cout << "Ice copy constructed\n";
}

Ice& Ice::operator=(const Ice& other) {
    std::cout << "Ice assigned\n";
    AMateria::operator=(other);
    return *this;
}

Ice::~Ice() {
    std::cout << "Ice destructed\n";
}

AMateria* Ice::clone() const {
    return new Ice(*this);
}

void Ice::use(ICharacter& target) {
    std::cout << "* shoots an ice bolt at " << target.getName() << " *\n";
}

