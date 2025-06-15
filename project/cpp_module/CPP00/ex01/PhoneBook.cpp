#include "PhoneBook.hpp"
#include <iostream>
#include <limits>
#include <iomanip>

PhoneBook::PhoneBook() : count(0), index(0) {}

void PhoneBook::addContact() {
    std::string fn, ln, nn, pn, ds;

    std::cout << "Enter first name: ";
    std::getline(std::cin, fn);
    std::cout << "Enter last name: ";
    std::getline(std::cin, ln);
    std::cout << "Enter nickname: ";
    std::getline(std::cin, nn);
    std::cout << "Enter phone number: ";
    std::getline(std::cin, pn);
    std::cout << "Enter darkest secret: ";
    std::getline(std::cin, ds);

    if (fn.empty() || ln.empty() || nn.empty() || pn.empty() || ds.empty()) {
        std::cout << "Error: All fields must be non-empty.\n";
        return;
    }

    contacts[index % 8].setContact(fn, ln, nn, pn, ds);
    index++;
    if (count < 8)
        count++;
    std::cout << "Contact added successfully.\n";
}

void PhoneBook::searchContact() const {
    if (count == 0) {
        std::cout << "PhoneBook is empty.\n";
        return;
    }

    std::cout << std::setw(10) << "Index" << "|"
              << std::setw(10) << "First Name" << "|"
              << std::setw(10) << "Last Name" << "|"
              << std::setw(10) << "Nickname" << "\n";

    for (int i = 0; i < count; i++) {
        contacts[i].displaySummary(i);
    }

    std::cout << "Enter index to view details: ";
    int idx;
    if (!(std::cin >> idx)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid index.\n";
        return;
    }
    std::cin.ignore(); // consume newline

    if (idx < 0 || idx >= count) {
        std::cout << "Index out of range.\n";
        return;
    }

    contacts[idx].displayFull();
}
