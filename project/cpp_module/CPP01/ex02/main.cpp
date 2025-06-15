#include <iostream>
#include <string>

int main() {
    std::string brain = "HI THIS IS BRAIN";

    std::string* stringPTR = &brain;
    std::string& stringREF = brain;

    // 打印地址
    std::cout << "Memory address of the string variable: " << &brain << std::endl;
    std::cout << "Memory address held by stringPTR:      " << stringPTR << std::endl;
    std::cout << "Memory address held by stringREF:      " << &stringREF << std::endl;

    // 打印值
    std::cout << "Value of the string variable:          " << brain << std::endl;
    std::cout << "Value pointed to by stringPTR:         " << *stringPTR << std::endl;
    std::cout << "Value pointed to by stringREF:         " << stringREF << std::endl;

    return 0;
}
