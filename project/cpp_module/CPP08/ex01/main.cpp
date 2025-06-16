#include <iostream>
#include "Span.hpp"
#include <cstdlib>
#include <ctime>

int main() {
    try {
        Span sp = Span(5);
        sp.addNumber(6);
        sp.addNumber(3);
        sp.addNumber(17);
        sp.addNumber(9);
        sp.addNumber(11);
        std::cout << "Shortest Span: " << sp.shortestSpan() << std::endl;
        std::cout << "Longest Span: " << sp.longestSpan() << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    std::cout << "----- Massive Span Test -----" << std::endl;
    try {
        Span big(10000);
        std::srand(std::time(0));
        std::vector<int> v(10000);
        for (size_t i = 0; i < v.size(); ++i) {
            v[i] = std::rand();
        }

        big.addNumber(v.begin(), v.end());

        std::cout << "Shortest Span (big): " << big.shortestSpan() << std::endl;
        std::cout << "Longest Span (big): " << big.longestSpan() << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Exception (big): " << e.what() << std::endl;
    }

    return 0;
}

