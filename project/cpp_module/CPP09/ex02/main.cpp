#include "PmergeMe.hpp"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Error\n";
        return 1;
    }

    PmergeMe sorter;
    if (!sorter.parseInput(argc, argv)) {
        std::cout << "Error\n";
        return 1;
    }

    sorter.printBefore();

    sorter.sortVector();
    sorter.sortDeque();

    sorter.printAfter();

    sorter.printTiming();

    return 0;
}

