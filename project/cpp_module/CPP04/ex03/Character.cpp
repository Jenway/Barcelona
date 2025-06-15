#include "Character.hpp"
#include <iostream>

Character::Character(std::string const & name) : name(name) {
    std::cout << "Character " << name << " constructed\n";
    for (int i = 0; i < 4; i++)
        inventory[i] = NULL;
}

Character::Character(const Character& other) : name(other.name) {
    std::cout << "Character copy constructed\n";
    for (int i = 0; i < 4; i++)
        inventory[i] = NULL;
    copyInventory(other);
}

Character& Character::operator=(const Character& other) {
    std::cout << "Character assigned\n";
    if (this != &other) {
        name = other.name;
        clearInventory();
        copyInventory(other);
    }
    return *this;
}

Character::~Character() {
    clearInventory();
    std::cout << "Character " << name << " destructed\n";
}

std::string const & Character::getName() const {
    return name;
}

void Character::equip(AMateria* m) {
    if (!m)
        return;
    for (int i = 0; i < 4; i++) {
        if (inventory[i] == NULL) {
            inventory[i] = m;
            std::cout << name << " equipped materia of type " << m->getType() << " in slot " << i << "\n";
            return;
        }
    }
    std::cout << name << "'s inventory full, can't equip materia\n";
}

void Character::unequip(int idx) {
    if (idx < 0 || idx >= 4)
        return;
    if (inventory[idx] != NULL) {
        std::cout << name << " unequipped materia of type " << inventory[idx]->getType() << " from slot " << idx << "\n";
        inventory[idx] = NULL;
    }
}

void Character::use(int idx, ICharacter& target) {
    if (idx < 0 || idx >= 4 || inventory[idx] == NULL)
        return;
    inventory[idx]->use(target);
}

void Character::clearInventory() {
    for (int i = 0; i < 4; i++) {
        delete inventory[i];
        inventory[i] = NULL;
    }
}

void Character::copyInventory(const Character& other) {
    for (int i = 0; i < 4; i++) {
        if (other.inventory[i] != NULL)
            inventory[i] = other.inventory[i]->clone();
        else
            inventory[i] = NULL;
    }
}

