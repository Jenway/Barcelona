#ifndef PHONEBOOK_HPP
#define PHONEBOOK_HPP

#include "Contact.hpp"

class PhoneBook {
private:
    Contact contacts[8];
    int count;
    int index;

public:
    PhoneBook();
    void addContact();
    void searchContact() const;
};

#endif
