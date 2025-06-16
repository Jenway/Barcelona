#include "BitcoinExchange.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>

BitcoinExchange::BitcoinExchange(const std::string& dbFile) {
    parseDb(dbFile);
}

void BitcoinExchange::parseDb(const std::string& dbFile) {
    std::ifstream file(dbFile.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Error: could not open exchange rate database.");
    }

    std::string line;
    std::getline(file, line); // Skip header

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string date, priceStr;

        if (!std::getline(ss, date, ',') || !std::getline(ss, priceStr))
            continue;

        float price = std::strtof(priceStr.c_str(), NULL);
        _exchangeRates[date] = price;
    }
}

float BitcoinExchange::getRate(const std::string& date) const {
    if (!isValidDate(date)) {
        throw std::invalid_argument("Error: bad input => " + date);
    }

    std::map<std::string, float>::const_iterator it = _exchangeRates.lower_bound(date);
    if (it == _exchangeRates.end() || it->first != date) {
        if (it == _exchangeRates.begin()) {
            throw std::invalid_argument("Error: date not in database => " + date);
        }
        --it;
    }
    return it->second;
}

bool BitcoinExchange::isValidDate(const std::string& date) const {
    if (date.length() != 10 || date[4] != '-' || date[7] != '-') return false;
    int y, m, d;
    char dash1, dash2;
    std::istringstream ss(date);
    ss >> y >> dash1 >> m >> dash2 >> d;
    return ss && dash1 == '-' && dash2 == '-' &&
           (y >= 2009 && m >= 1 && m <= 12 && d >= 1 && d <= 31);
}

