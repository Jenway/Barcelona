#include "DiamondTrap.hpp"

int main() {
    std::cout << "--- DiamondTrap Test ---" << std::endl;

    DiamondTrap d("Diamondy");
    d.attack("target");
    d.whoAmI();

    d.takeDamage(30);
    d.beRepaired(20);
    d.guardGate();        // 来自 ScavTrap
    d.highFivesGuys();    // 来自 FragTrap

    std::cout << "--- Destructor Check ---" << std::endl;
    return 0;
}
