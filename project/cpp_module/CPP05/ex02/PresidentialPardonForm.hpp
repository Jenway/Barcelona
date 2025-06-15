#ifndef PRESIDENTIALPARDONFORM_HPP
#define PRESIDENTIALPARDONFORM_HPP

#include "AForm.hpp"

class PresidentialPardonForm : public AForm {
public:
    PresidentialPardonForm(const std::string& target);
    virtual ~PresidentialPardonForm();

    void execute(Bureaucrat const & executor) const;

private:
    PresidentialPardonForm();
    PresidentialPardonForm(const PresidentialPardonForm&);
    PresidentialPardonForm& operator=(const PresidentialPardonForm&);
};

#endif

