#include <iostream>
#include "Base.hpp"

Base* generate();
void identify(Base* p);
void identify(Base& p);

int main() {
    Base* obj = generate();

    std::cout << "Using pointer:\n";
    identify(obj);

    std::cout << "Using reference:\n";
    identify(*obj);

    delete obj;
    return 0;
}

