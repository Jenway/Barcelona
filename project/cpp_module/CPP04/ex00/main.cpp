#include "Animal.hpp"
#include "Dog.hpp"
#include "Cat.hpp"
#include "WrongAnimal.hpp"
#include "WrongCat.hpp"

int main() {
    std::cout << "--- Correct hierarchy ---\n";
    const Animal* meta = new Animal();
    const Animal* j = new Dog();
    const Animal* i = new Cat();

    std::cout << j->getType() << "\n";
    std::cout << i->getType() << "\n";

    i->makeSound(); // Should call Cat::makeSound
    j->makeSound(); // Should call Dog::makeSound
    meta->makeSound(); // Should call Animal::makeSound

    delete meta;
    delete j;
    delete i;

    std::cout << "\n--- Wrong hierarchy ---\n";
    const WrongAnimal* wrong = new WrongAnimal();
    const WrongAnimal* wrongCat = new WrongCat();

    std::cout << wrongCat->getType() << "\n";
    wrongCat->makeSound(); // ⚠️ Will call WrongAnimal::makeSound due to lack of virtual
    wrong->makeSound();

    delete wrong;
    delete wrongCat;

    return 0;
}

