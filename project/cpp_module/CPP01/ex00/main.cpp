#include "Zombie.hpp"

int main() {
    Zombie* heapZombie = newZombie("Heapster");
    heapZombie->announce();
    delete heapZombie;

    randomChump("Stacky");
    return 0;
}
