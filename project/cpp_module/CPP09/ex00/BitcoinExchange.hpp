#ifndef BITCOINEXCHANGE_HPP
#define BITCOINEXCHANGE_HPP

#include <map>
#include <string>

class BitcoinExchange {
public:
    BitcoinExchange(const std::string& dbFile);
    float getRate(const std::string& date) const;

private:
    std::map<std::string, float> _exchangeRates;
    void parseDb(const std::string& dbFile);
    bool isValidDate(const std::string& date) const;
};

#endif

