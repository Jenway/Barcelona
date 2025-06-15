#include "Intern.hpp"
#include <iostream>

int main() {
    Intern someRandomIntern;

    AForm* form1 = someRandomIntern.makeForm("robotomy request", "Bender");
    if (form1) {
        std::cout << *form1 << std::endl;
        delete form1;
    }

    AForm* form2 = someRandomIntern.makeForm("shrubbery creation", "home");
    if (form2) {
        std::cout << *form2 << std::endl;
        delete form2;
    }

    AForm* form3 = someRandomIntern.makeForm("unknown form", "target");
    if (form3) {
        delete form3;
    }

    return 0;
}

