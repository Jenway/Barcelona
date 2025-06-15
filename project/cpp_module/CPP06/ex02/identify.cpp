#include <iostream>
#include <cstdlib>
#include <ctime>
#include "Base.hpp"
#include "A.hpp"
#include "B.hpp"
#include "C.hpp"

Base* generate() {
    std::srand(std::time(NULL)); // seed random once per run
    int r = std::rand() % 3;
    switch (r) {
        case 0: std::cout << "Generated: A\n"; return new A;
        case 1: std::cout << "Generated: B\n"; return new B;
        case 2: std::cout << "Generated: C\n"; return new C;
    }
    return NULL;
}

void identify(Base* p) {
    if (dynamic_cast<A*>(p)) std::cout << "Type: A\n";
    else if (dynamic_cast<B*>(p)) std::cout << "Type: B\n";
    else if (dynamic_cast<C*>(p)) std::cout << "Type: C\n";
    else std::cout << "Unknown Type\n";
}

void identify(Base& p) {
    try {
        (void)dynamic_cast<A&>(p);
        std::cout << "Type: A\n";
    } catch (...) {
        try {
            (void)dynamic_cast<B&>(p);
            std::cout << "Type: B\n";
        } catch (...) {
            try {
                (void)dynamic_cast<C&>(p);
                std::cout << "Type: C\n";
            } catch (...) {
                std::cout << "Unknown Type\n";
            }
        }
    }
}

