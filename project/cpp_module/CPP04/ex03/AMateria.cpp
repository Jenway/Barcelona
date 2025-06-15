#include "AMateria.hpp"
#include "ICharacter.hpp"

AMateria::AMateria(std::string const & type) : type(type) {
    std::cout << "AMateria constructed: " << type << std::endl;
}

AMateria::AMateria(const AMateria& other) : type(other.type) {
    std::cout << "AMateria copy constructed: " << type << std::endl;
}

AMateria& AMateria::operator=(const AMateria& other) {
    std::cout << "AMateria assigned\n";
    if (this != &other)
        type = other.type;
    return *this;
}

AMateria::~AMateria() {
    std::cout << "AMateria destructed: " << type << std::endl;
}

std::string const & AMateria::getType() const {
    return type;
}

void AMateria::use(ICharacter& target) {
    (void)target;
    std::cout << "* uses AMateria on " << target.getName() << " *\n";
}

