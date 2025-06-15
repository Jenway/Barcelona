#ifndef HUMANB_HPP
#define HUMANB_HPP

#include <string>
#include "Weapon.hpp"

class HumanB {
private:
    std::string name;
    Weapon* weapon; // 指针

public:
    HumanB(const std::string& name);
    ~HumanB();

    void setWeapon(Weapon& weapon);
    void attack() const;
};

#endif
