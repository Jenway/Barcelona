#include "Intern.hpp"
#include "ShrubberyCreationForm.hpp"
#include "RobotomyRequestForm.hpp"
#include "PresidentialPardonForm.hpp"
#include <iostream>

// 构造、析构、赋值
Intern::Intern() {}
Intern::Intern(const Intern& other) { (void)other; }
Intern& Intern::operator=(const Intern& other) { (void)other; return *this; }
Intern::~Intern() {}
// 结构体将表单名与对应构造函数绑定
typedef AForm* (*FormConstructor)(const std::string& target);

struct FormPair {
    const char* name;
    FormConstructor constructor;
};

// 三个表单的构造函数封装为静态函数，方便统一调用
static AForm* createShrubbery(const std::string& target) {
    return new ShrubberyCreationForm(target);
}
static AForm* createRobotomy(const std::string& target) {
    return new RobotomyRequestForm(target);
}
static AForm* createPresidential(const std::string& target) {
    return new PresidentialPardonForm(target);
}


AForm* Intern::makeForm(const std::string& formName, const std::string& target) const {
    FormPair forms[] = {
        {"shrubbery creation", createShrubbery},
        {"robotomy request", createRobotomy},
        {"presidential pardon", createPresidential}
    };

    for (size_t i = 0; i < sizeof(forms) / sizeof(forms[0]); ++i) {
        if (formName == forms[i].name) {
            std::cout << "Intern creates " << forms[i].name << std::endl;
            return forms[i].constructor(target);
        }
    }

    std::cerr << "Intern could not create form because form name \"" << formName << "\" is unknown." << std::endl;
    return NULL;
}

const char* Intern::UnknownFormException::what() const throw() {
    return "Intern: unknown form requested";
}

