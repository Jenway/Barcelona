#include "Contact.hpp"
#include <iostream>
#include <iomanip>

Contact::Contact() {}

void Contact::setContact(const std::string& fn, const std::string& ln,
                         const std::string& nn, const std::string& pn,
                         const std::string& ds) {
    firstName = fn;
    lastName = ln;
    nickname = nn;
    phoneNumber = pn;
    darkestSecret = ds;
}

bool Contact::isEmpty() const {
    return firstName.empty();
}

static std::string formatField(const std::string& str) {
    if (str.length() > 10)
        return str.substr(0, 9) + ".";
    return str;
}

void Contact::displaySummary(int index) const {
    std::cout << std::setw(10) << index << "|"
              << std::setw(10) << formatField(firstName) << "|"
              << std::setw(10) << formatField(lastName) << "|"
              << std::setw(10) << formatField(nickname) << "\n";
}

void Contact::displayFull() const {
    std::cout << "First name: " << firstName << "\n"
              << "Last name: " << lastName << "\n"
              << "Nickname: " << nickname << "\n"
              << "Phone number: " << phoneNumber << "\n"
              << "Darkest secret: " << darkestSecret << "\n";
}
