#include "Bureaucrat.hpp"
#include "AForm.hpp"
#include "ShrubberyCreationForm.hpp"
#include "RobotomyRequestForm.hpp"
#include "PresidentialPardonForm.hpp"

int main() {
    Bureaucrat john("John", 1);
    Bureaucrat jim("Jim", 150);

    ShrubberyCreationForm shrub("home");
    RobotomyRequestForm robot("Bender");
    PresidentialPardonForm pardon("Marvin");

    // 签署表单
    try {
        shrub.beSigned(john);
        robot.beSigned(john);
        pardon.beSigned(john);
    } catch (std::exception& e) {
        std::cout << "Signing error: " << e.what() << std::endl;
    }

    // 执行表单
    john.executeForm(shrub);
    john.executeForm(robot);
    john.executeForm(pardon);

    jim.executeForm(shrub); // 权限不足
    jim.executeForm(robot); // 权限不足
    jim.executeForm(pardon); // 权限不足

    return 0;
}

