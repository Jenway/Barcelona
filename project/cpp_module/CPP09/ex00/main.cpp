#include "BitcoinExchange.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

static bool isValidValue(const std::string& valueStr, float& value) {
    char* end = NULL;
    value = std::strtof(valueStr.c_str(), &end);
    if (*end != '\0') return false;
    return (value >= 0.0f && value <= 1000.0f);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Error: could not open file." << std::endl;
        return 1;
    }

    std::ifstream input(argv[1]);
    if (!input.is_open()) {
        std::cerr << "Error: could not open file." << std::endl;
        return 1;
    }

    BitcoinExchange btc("data.csv"); // your DB file

    std::string line;
    std::getline(input, line); // skip header
    while (std::getline(input, line)) {
        size_t sep = line.find('|');
        if (sep == std::string::npos) {
            std::cerr << "Error: bad input => " << line << std::endl;
            continue;
        }

        std::string date = line.substr(0, sep);
        std::string valueStr = line.substr(sep + 1);

        date.erase(date.find_last_not_of(" \t") + 1);
        valueStr.erase(0, valueStr.find_first_not_of(" \t"));

        float value;
        if (!isValidValue(valueStr, value)) {
            if (valueStr.find('-') != std::string::npos)
                std::cerr << "Error: not a positive number." << std::endl;
            else
                std::cerr << "Error: too large a number." << std::endl;
            continue;
        }

        try {
            float rate = btc.getRate(date);
            std::cout << date << " => " << value << " = " << rate * value << std::endl;
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    return 0;
}

