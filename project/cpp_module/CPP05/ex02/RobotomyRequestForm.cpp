#include "RobotomyRequestForm.hpp"
#include <cstdlib>
#include <ctime>
#include <iostream>

RobotomyRequestForm::RobotomyRequestForm(const std::string& target)
    : AForm("RobotomyRequestForm", 72, 45, target) {
    std::srand(std::time(NULL));
}

RobotomyRequestForm::~RobotomyRequestForm() {}

void RobotomyRequestForm::execute(Bureaucrat const & executor) const {
    checkExecution(executor);

    std::cout << "* drilling noises *" << std::endl;

    if (std::rand() % 2)
        std::cout << getTarget() << " has been robotomized successfully." << std::endl;
    else
        std::cout << "Robotomy failed on " << getTarget() << "." << std::endl;
}

