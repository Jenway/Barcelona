#include <iostream>
#include "iter.hpp"

template <typename T>
void printElement(const T& element) {
    std::cout << element << std::endl;
}

void toUpper(char& c) {
    if ('a' <= c && c <= 'z')
        c = c - 'a' + 'A';
}

int main() {
    int arr[] = {1, 2, 3, 4};
    std::cout << "Int array:\n";
    iter(arr, 4, printElement<int>);

    std::string strArr[] = {"hello", "world", "!"};
    std::cout << "\nString array:\n";
    iter(strArr, 3, printElement<std::string>);

    char charArr[] = {'a', 'b', 'c', 'd'};
    std::cout << "\nChar array before toUpper:\n";
    iter(charArr, 4, printElement<char>);

    iter(charArr, 4, toUpper);
    std::cout << "\nChar array after toUpper:\n";
    iter(charArr, 4, printElement<char>);

    return 0;
}

