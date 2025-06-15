#include "Bureaucrat.hpp"
#include "Form.hpp"

int main() {
    try {
        Bureaucrat alice("Alice", 42);
        Form taxForm("Tax Form", 40, 100);

        std::cout << taxForm << std::endl;
        alice.signForm(taxForm); // too low
        std::cout << taxForm << std::endl;

        Bureaucrat bob("Bob", 10);
        bob.signForm(taxForm); // should work now
        std::cout << taxForm << std::endl;
    } catch (std::exception& e) {
        std::cout << "Unhandled exception: " << e.what() << std::endl;
    }

    try {
        Form invalidForm("Fail Form", 0, 160); // should throw
    } catch (std::exception& e) {
        std::cout << "Exception on creation: " << e.what() << std::endl;
    }

    return 0;
}

