#include "AForm.hpp"
#include "Bureaucrat.hpp"

AForm::AForm(const std::string& name, int gradeToSign, int gradeToExec, const std::string& target)
    : _name(name), _signed(false), _gradeToSign(gradeToSign), _gradeToExec(gradeToExec), _target(target) {
    if (gradeToSign < 1 || gradeToExec < 1)
        throw GradeTooHighException();
    if (gradeToSign > 150 || gradeToExec > 150)
        throw GradeTooLowException();
}

AForm::AForm(const AForm& other)
    : _name(other._name),
      _signed(other._signed),
      _gradeToSign(other._gradeToSign),
      _gradeToExec(other._gradeToExec),
      _target(other._target) {}

AForm& AForm::operator=(const AForm& other) {
    if (this != &other) {
        _signed = other._signed;
        // const members no assign
    }
    return *this;
}

AForm::~AForm() {}

const std::string& AForm::getName() const { return _name; }
bool AForm::isSigned() const { return _signed; }
int AForm::getGradeToSign() const { return _gradeToSign; }
int AForm::getGradeToExec() const { return _gradeToExec; }
const std::string& AForm::getTarget() const { return _target; }

void AForm::beSigned(const Bureaucrat& b) {
    if (b.getGrade() > _gradeToSign)
        throw GradeTooLowException();
    _signed = true;
}

const char* AForm::GradeTooHighException::what() const throw() {
    return "Form grade too high!";
}

const char* AForm::GradeTooLowException::what() const throw() {
    return "Form grade too low!";
}

const char* AForm::FormNotSignedException::what() const throw() {
    return "Form is not signed!";
}

void AForm::checkExecution(const Bureaucrat& executor) const {
    if (!_signed)
        throw FormNotSignedException();
    if (executor.getGrade() > _gradeToExec)
        throw GradeTooLowException();
}

std::ostream& operator<<(std::ostream& os, const AForm& f) {
    os << "Form \"" << f.getName() << "\", target: " << f.getTarget()
       << ", signed: " << std::boolalpha << f.isSigned()
       << ", grade to sign: " << f.getGradeToSign()
       << ", grade to execute: " << f.getGradeToExec();
    return os;
}

