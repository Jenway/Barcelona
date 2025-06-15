#include "Form.hpp"
#include "Bureaucrat.hpp"

// Canonical
Form::Form() : _name("Unnamed"), _signed(false), _gradeToSign(150), _gradeToExecute(150) {}

Form::Form(const std::string& name, int gradeToSign, int gradeToExecute)
    : _name(name), _signed(false), _gradeToSign(gradeToSign), _gradeToExecute(gradeToExecute) {
    if (gradeToSign < 1 || gradeToExecute < 1)
        throw Form::GradeTooHighException();
    if (gradeToSign > 150 || gradeToExecute > 150)
        throw Form::GradeTooLowException();
}

Form::Form(const Form& other)
    : _name(other._name),
      _signed(other._signed),
      _gradeToSign(other._gradeToSign),
      _gradeToExecute(other._gradeToExecute) {}

Form& Form::operator=(const Form& other) {
    if (this != &other) {
        _signed = other._signed;
        // const variables are not assignable
    }
    return *this;
}

Form::~Form() {}

// Getters
const std::string& Form::getName() const { return _name; }
bool Form::isSigned() const { return _signed; }
int Form::getGradeToSign() const { return _gradeToSign; }
int Form::getGradeToExecute() const { return _gradeToExecute; }

// beSigned
void Form::beSigned(const Bureaucrat& b) {
    if (b.getGrade() > _gradeToSign)
        throw Form::GradeTooLowException();
    _signed = true;
}

// Exceptions
const char* Form::GradeTooHighException::what() const throw() {
    return "Form grade too high!";
}
const char* Form::GradeTooLowException::what() const throw() {
    return "Form grade too low!";
}

// operator<<
std::ostream& operator<<(std::ostream& os, const Form& f) {
    os << "Form \"" << f.getName() << "\", signed: " << std::boolalpha << f.isSigned()
       << ", grade to sign: " << f.getGradeToSign()
       << ", grade to execute: " << f.getGradeToExecute();
    return os;
}

