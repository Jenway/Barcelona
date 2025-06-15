#include "ScalarConverter.hpp"

#include <iostream>
#include <cstdlib>   // strtol, strtod, atof
#include <limits>
#include <cmath>     // isnan, isinf
#include <cerrno>

ScalarConverter::ScalarConverter() {}
ScalarConverter::~ScalarConverter() {}
ScalarConverter::ScalarConverter(const ScalarConverter&) {}
ScalarConverter& ScalarConverter::operator=(const ScalarConverter&) { return *this; }

bool ScalarConverter::isPseudoLiteral(const std::string& s) {
    return (s == "nan" || s == "nanf" ||
            s == "+inf" || s == "+inff" ||
            s == "-inf" || s == "-inff");
}

bool ScalarConverter::isChar(const std::string& s) {
    return s.length() == 1 && !isdigit(s[0]);
}

bool ScalarConverter::isInt(const std::string& s) {
    char* endptr = NULL;
    errno = 0;
    long val = strtol(s.c_str(), &endptr, 10);
    return (*endptr == '\0' && errno == 0 && val >= std::numeric_limits<int>::min() && val <= std::numeric_limits<int>::max());
}

bool ScalarConverter::isFloat(const std::string& s) {
    if (s.empty())
        return false;
    if (isPseudoLiteral(s))
        return s.find('f') != std::string::npos; // nanf, +inff etc.

    size_t len = s.length();
    if (s[len - 1] != 'f')
        return false;
    char* endptr = NULL;
    errno = 0;
    strtof(s.c_str(), &endptr);
    return (endptr == s.c_str() + len - 1) && errno == 0;
}

bool ScalarConverter::isDouble(const std::string& s) {
    if (isPseudoLiteral(s))
        return s.find('f') == std::string::npos; // nan, +inf, -inf (no f)

    char* endptr = NULL;
    errno = 0;
    strtod(s.c_str(), &endptr);
    return (*endptr == '\0' && errno == 0);
}

// 输出辅助
void ScalarConverter::printChar(char c) {
    if (std::isprint(static_cast<unsigned char>(c)))
        std::cout << "char: '" << c << "'" << std::endl;
    else
        std::cout << "char: Non displayable" << std::endl;
}

void ScalarConverter::printInt(int i) {
    std::cout << "int: " << i << std::endl;
}

void ScalarConverter::printFloat(float f) {
    std::cout << "float: " << f;
    if (f - static_cast<int>(f) == 0)
        std::cout << ".0";
    std::cout << "f" << std::endl;
}

void ScalarConverter::printDouble(double d) {
    std::cout << "double: " << d;
    if (d - static_cast<int>(d) == 0)
        std::cout << ".0";
    std::cout << std::endl;
}

void ScalarConverter::printImpossible() {
    std::cout << "impossible" << std::endl;
}

void ScalarConverter::convert(const std::string& literal) {
    // char 类型
    if (isChar(literal)) {
        char c = literal[0];
        printChar(c);
        printInt(static_cast<int>(c));
        printFloat(static_cast<float>(c));
        printDouble(static_cast<double>(c));
        return;
    }

    // 伪字面值 nan, inf等
    if (isPseudoLiteral(literal)) {
        // 处理 nan, nanf
        bool isFloatLiteral = (literal.find('f') != std::string::npos);

        std::cout << "char: impossible" << std::endl;
        std::cout << "int: impossible" << std::endl;

        if (isFloatLiteral)
            std::cout << "float: " << literal << std::endl;
        else
            std::cout << "float: " << literal << "f" << std::endl;

        if (isFloatLiteral)
            std::cout << "double: " << literal.substr(0, literal.size() - 1) << std::endl;
        else
            std::cout << "double: " << literal << std::endl;

        return;
    }

    // int 类型
    if (isInt(literal)) {
        int i = std::atoi(literal.c_str());

        if (i < 0 || i > 127)
            std::cout << "char: impossible" << std::endl;
        else if (std::isprint(static_cast<unsigned char>(i)))
            std::cout << "char: '" << static_cast<char>(i) << "'" << std::endl;
        else
            std::cout << "char: Non displayable" << std::endl;

        printInt(i);
        printFloat(static_cast<float>(i));
        printDouble(static_cast<double>(i));
        return;
    }

    // float 类型
    if (isFloat(literal)) {
        float f = std::strtof(literal.c_str(), NULL);
        if (f < 0 || f > 127)
            std::cout << "char: impossible" << std::endl;
        else if (std::isprint(static_cast<unsigned char>(f)))
            std::cout << "char: '" << static_cast<char>(f) << "'" << std::endl;
        else
            std::cout << "char: Non displayable" << std::endl;

        if (f > static_cast<float>(std::numeric_limits<int>::max()) || f < static_cast<float>(std::numeric_limits<int>::min()))
            std::cout << "int: impossible" << std::endl;
        else
            printInt(static_cast<int>(f));

        printFloat(f);
        printDouble(static_cast<double>(f));
        return;
    }

    // double 类型
    if (isDouble(literal)) {
        double d = std::strtod(literal.c_str(), NULL);
        if (d < 0 || d > 127)
            std::cout << "char: impossible" << std::endl;
        else if (std::isprint(static_cast<unsigned char>(d)))
            std::cout << "char: '" << static_cast<char>(d) << "'" << std::endl;
        else
            std::cout << "char: Non displayable" << std::endl;

        if (d > static_cast<double>(std::numeric_limits<int>::max()) || d < static_cast<double>(std::numeric_limits<int>::min()))
            std::cout << "int: impossible" << std::endl;
        else
            printInt(static_cast<int>(d));

        if (d > static_cast<double>(std::numeric_limits<float>::max()) || d < -static_cast<double>(std::numeric_limits<float>::max()))
            std::cout << "float: impossible" << std::endl;
        else
            printFloat(static_cast<float>(d));

        printDouble(d);
        return;
    }

    // 无法识别类型
    std::cout << "char: impossible" << std::endl;
    std::cout << "int: impossible" << std::endl;
    std::cout << "float: impossible" << std::endl;
    std::cout << "double: impossible" << std::endl;
}

