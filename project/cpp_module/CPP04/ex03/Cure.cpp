#include "Cure.hpp"
#include "ICharacter.hpp"
#include <iostream>

Cure::Cure() : AMateria("cure") {
    std::cout << "Cure constructed\n";
}

Cure::Cure(const Cure& other) : AMateria(other) {
    std::cout << "Cure copy constructed\n";
}

Cure& Cure::operator=(const Cure& other) {
    std::cout << "Cure assigned\n";
    AMateria::operator=(other);
    return *this;
}

Cure::~Cure() {
    std::cout << "Cure destructed\n";
}

AMateria* Cure::clone() const {
    return new Cure(*this);
}

void Cure::use(ICharacter& target) {
    std::cout << "* heals " << target.getName() << "'s wounds *\n";
}

