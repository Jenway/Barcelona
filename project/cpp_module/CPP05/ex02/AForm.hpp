#ifndef AFORM_HPP
#define AFORM_HPP

#include <iostream>
#include <string>
#include <exception>
#include "Bureaucrat.hpp"

class Bureaucrat;

class AForm {
private:
    const std::string _name;
    bool _signed;
    const int _gradeToSign;
    const int _gradeToExec;
    const std::string _target;

public:
    // Canonical
    AForm(const std::string& name, int gradeToSign, int gradeToExec, const std::string& target);
    AForm(const AForm& other);
    AForm& operator=(const AForm& other);
    virtual ~AForm();

    // Getters
    const std::string& getName() const;
    bool isSigned() const;
    int getGradeToSign() const;
    int getGradeToExec() const;
    const std::string& getTarget() const;

    void beSigned(const Bureaucrat& b);

    // Exceptions
    class GradeTooHighException : public std::exception {
    public:
        virtual const char* what() const throw();
    };
    class GradeTooLowException : public std::exception {
    public:
        virtual const char* what() const throw();
    };
    class FormNotSignedException : public std::exception {
    public:
        virtual const char* what() const throw();
    };

    // Abstract execute method
    virtual void execute(Bureaucrat const & executor) const = 0;

protected:
    void checkExecution(const Bureaucrat& executor) const; // utility for derived classes

};

std::ostream& operator<<(std::ostream& os, const AForm& f);

#endif

