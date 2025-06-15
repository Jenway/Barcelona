#include "Bureaucrat.hpp"

int main() {
    try {
        Bureaucrat a("Alice", 1);
        std::cout << a << std::endl;
        a.incrementGrade(); // should throw
    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    try {
        Bureaucrat b("Bob", 150);
        std::cout << b << std::endl;
        b.decrementGrade(); // should throw
    } catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    try {
        Bureaucrat c("Charlie", 0); // invalid
    } catch (std::exception& e) {
        std::cout << "Exception on creation: " << e.what() << std::endl;
    }

    try {
        Bureaucrat d("Diana", 151); // invalid
    } catch (std::exception& e) {
        std::cout << "Exception on creation: " << e.what() << std::endl;
    }

    return 0;
}
