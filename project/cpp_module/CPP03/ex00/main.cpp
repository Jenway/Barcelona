#include "ClapTrap.hpp"

int main() {
    ClapTrap bot1("Alpha");
    ClapTrap bot2("Beta");

    bot1.attack("Beta");
    bot2.takeDamage(3);

    bot2.beRepaired(2);
    bot2.attack("Alpha");
    bot1.takeDamage(5);

    bot1.beRepaired(10);

    // Edge case: reduce energy to 0
    for (int i = 0; i < 10; ++i)
        bot1.attack("Beta");

    bot1.attack("Beta");  // Should print no energy
    bot1.beRepaired(5);   // Should print no energy

    return 0;
}

