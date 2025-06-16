#include <iostream>
#include <vector>
#include <list>
#include "easyfind.hpp"

void testVector() {
    std::vector<int> v;
    v.push_back(1);
    v.push_back(3);
    v.push_back(5);
    v.push_back(7);
    v.push_back(9);

    try {
        std::vector<int>::iterator it = easyfind(v, 5);
        std::cout << "Found 5 in vector at index: " << std::distance(v.begin(), it) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Vector test: " << e.what() << std::endl;
    }

    try {
        easyfind(v, 6);
    } catch (const std::exception& e) {
        std::cerr << "Vector test: " << e.what() << std::endl;
    }
}

void testList() {
    std::list<int> l;
    for (int i = 10; i <= 50; i += 10)
        l.push_back(i);

    try {
        std::list<int>::iterator it = easyfind(l, 30);
        std::cout << "Found 30 in list at position: ";
        for (std::list<int>::iterator it2 = l.begin(); it2 != it; ++it2)
            std::cout << *it2 << " -> ";
        std::cout << *it << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "List test: " << e.what() << std::endl;
    }

    try {
        easyfind(l, 25);
    } catch (const std::exception& e) {
        std::cerr << "List test: " << e.what() << std::endl;
    }
}

int main() {
    testVector();
    std::cout << "-----" << std::endl;
    testList();
    return 0;
}

