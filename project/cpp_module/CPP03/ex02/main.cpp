#include "ClapTrap.hpp"
#include "ScavTrap.hpp"
#include "FragTrap.hpp"

int main() {
    std::cout << "--- ClapTrap Test ---" << std::endl;
    ClapTrap c("CL4P-TP");
    c.attack("Bandit");
    c.beRepaired(2);
    c.takeDamage(5);

    std::cout << "\n--- ScavTrap Test ---" << std::endl;
    ScavTrap s("SC4V-TP");
    s.attack("Skag");
    s.guardGate();
    s.takeDamage(40);
    s.beRepaired(10);

    std::cout << "\n--- FragTrap Test ---" << std::endl;
    FragTrap f("FR4G-TP");
    f.attack("Psycho");
    f.takeDamage(20);
    f.beRepaired(5);
    f.highFivesGuys();

    std::cout << "\n--- Destructor Check ---" << std::endl;
    return 0;
}
