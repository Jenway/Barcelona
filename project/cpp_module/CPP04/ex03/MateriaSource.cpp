#include "MateriaSource.hpp"
#include <iostream>

MateriaSource::MateriaSource() {
    for (int i = 0; i < 4; i++)
        learned[i] = NULL;
    std::cout << "MateriaSource constructed\n";
}

MateriaSource::MateriaSource(const MateriaSource& other) {
    std::cout << "MateriaSource copy constructed\n";
    for (int i = 0; i < 4; i++)
        learned[i] = NULL;
    copy(other);
}

MateriaSource& MateriaSource::operator=(const MateriaSource& other) {
    std::cout << "MateriaSource assigned\n";
    if (this != &other) {
        clear();
        copy(other);
    }
    return *this;
}

MateriaSource::~MateriaSource() {
    clear();
    std::cout << "MateriaSource destructed\n";
}

void MateriaSource::clear() {
    for (int i = 0; i < 4; i++) {
        delete learned[i];
        learned[i] = NULL;
    }
}

void MateriaSource::copy(const MateriaSource& other) {
    for (int i = 0; i < 4; i++) {
        if (other.learned[i] != NULL)
            learned[i] = other.learned[i]->clone();
        else
            learned[i] = NULL;
    }
}

void MateriaSource::learnMateria(AMateria* m) {
    if (!m)
        return;
    for (int i = 0; i < 4; i++) {
        if (learned[i] == NULL) {
            learned[i] = m->clone();
            std::cout << "MateriaSource learned materia of type " << m->getType() << " in slot " << i << "\n";
            return;
        }
    }
    std::cout << "MateriaSource can't learn more materias, full\n";
}

AMateria* MateriaSource::createMateria(std::string const & type) {
    for (int i = 0; i < 4; i++) {
        if (learned[i] && learned[i]->getType() == type) {
            std::cout << "MateriaSource creates materia of type " << type << "\n";
            return learned[i]->clone();
        }
    }
    std::cout << "MateriaSource does not know materia of type " << type << "\n";
    return NULL;
}

