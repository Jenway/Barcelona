#include "Zombie.hpp"
#include <sstream>

std::string intToString(int n) {
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

Zombie* zombieHorde(int N, std::string name) {
    if (N <= 0)
        return NULL;

    Zombie* horde = new Zombie[N];
    for (int i = 0; i < N; ++i) {
        horde[i].setName(name + "_" + intToString(i + 1));
    }
    return horde;
}
