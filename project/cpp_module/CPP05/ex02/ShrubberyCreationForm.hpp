#ifndef SHRUBBERYCREATIONFORM_HPP
#define SHRUBBERYCREATIONFORM_HPP

#include "AForm.hpp"

class ShrubberyCreationForm : public AForm {
public:
    ShrubberyCreationForm(const std::string& target);
    virtual ~ShrubberyCreationForm();

    void execute(Bureaucrat const & executor) const;

private:
    ShrubberyCreationForm();
    ShrubberyCreationForm(const ShrubberyCreationForm&);
    ShrubberyCreationForm& operator=(const ShrubberyCreationForm&);
};

#endif

