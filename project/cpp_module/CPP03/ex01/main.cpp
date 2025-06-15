#include "ClapTrap.hpp"
#include "ScavTrap.hpp"

int main() {
    std::cout << "--- ClapTrap Test ---" << std::endl;
    ClapTrap a("Alpha");
    a.attack("Enemy");
    a.takeDamage(4);
    a.beRepaired(3);

    std::cout << "\n--- ScavTrap Test ---" << std::endl;
    ScavTrap s("Serena");
    s.attack("Intruder");
    s.takeDamage(30);
    s.beRepaired(10);
    s.guardGate();

    std::cout << "\n--- Destructor Check ---" << std::endl;
    return 0;
}
