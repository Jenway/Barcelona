#ifndef RPN_HPP
#define RPN_HPP

#include <string>

class RPN {
public:
    RPN();
    int calculate(const std::string& expression);
};

#endif

