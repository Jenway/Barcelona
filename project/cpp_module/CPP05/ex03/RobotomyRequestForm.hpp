#ifndef ROBOTOMYREQUESTFORM_HPP
#define ROBOTOMYREQUESTFORM_HPP

#include "AForm.hpp"

class RobotomyRequestForm : public AForm {
public:
    RobotomyRequestForm(const std::string& target);
    virtual ~RobotomyRequestForm();

    void execute(Bureaucrat const & executor) const;

private:
    RobotomyRequestForm();
    RobotomyRequestForm(const RobotomyRequestForm&);
    RobotomyRequestForm& operator=(const RobotomyRequestForm&);
};

#endif

