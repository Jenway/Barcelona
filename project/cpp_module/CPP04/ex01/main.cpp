#include "Animal.hpp"
#include "Dog.hpp"
#include "Cat.hpp"

int main() {
    const int size = 4;
    Animal* animals[size];

    // Create half dogs, half cats
    for (int i = 0; i < size / 2; i++)
        animals[i] = new Dog();
    for (int i = size / 2; i < size; i++)
        animals[i] = new Cat();

    // Test makeSound
    for (int i = 0; i < size; i++) {
        std::cout << animals[i]->getType() << " says: ";
        animals[i]->makeSound();
    }

    // Test deep copy
    Dog* dog1 = new Dog();
    dog1->getBrain()->setIdea(0, "Chasing cats");
    Dog* dog2 = new Dog(*dog1);
    dog2->getBrain()->setIdea(0, "Eating bones");

    std::cout << "Dog1 idea 0: " << dog1->getBrain()->getIdea(0) << std::endl;
    std::cout << "Dog2 idea 0: " << dog2->getBrain()->getIdea(0) << std::endl;

    delete dog1;
    delete dog2;

    // Cleanup
    for (int i = 0; i < size; i++)
        delete animals[i];

    return 0;
}

