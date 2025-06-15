#ifndef DIAMONDTRAP_HPP
#define DIAMONDTRAP_HPP

#include "FragTrap.hpp"
#include "ScavTrap.hpp"

class DiamondTrap : public ScavTrap, public FragTrap {
private:
    std::string _name;

public:
    DiamondTrap();
    DiamondTrap(const std::string& name);
    DiamondTrap(const DiamondTrap& other);
    DiamondTrap& operator=(const DiamondTrap& other);
    ~DiamondTrap();
    
    // using ScavTrap::attack; // 明确使用 ScavTrap 的 attack()
    void attack(const std::string& target) {
        ScavTrap::attack(target);  // 或 FragTrap::attack(target)
    }
    void whoAmI();
};

#endif
