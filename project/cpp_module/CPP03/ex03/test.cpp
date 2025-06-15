#include<iostream>

using namespace std;

class ClapTrap {
public:
    virtual void attack() { cout << "ClapTrap\n"; }
};

class ScavTrap : public virtual ClapTrap {
public:
    void attack() { cout << "ScavTrap\n"; }
};

class FragTrap : public  virtual ClapTrap {
public:
    void attack() { cout << "FragTrap\n"; }
};

class DiamondTrap : public ScavTrap, public FragTrap {
public:
    using ScavTrap::attack; // ❌ 无效！仍然报错
    // 你可以：
    /*
    void attack() {
        ScavTrap::attack();
    }
    */
};

int main() {
    DiamondTrap d;
    d.attack();
    ClapTrap* c = &d;
    c->attack();
}
