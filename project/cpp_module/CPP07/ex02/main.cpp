#include <iostream>
#include "Array.hpp"

int main() {
    try {
        Array<int> a(5);
        for (size_t i = 0; i < a.size(); ++i)
            a[i] = static_cast<int>(i * 10);

        std::cout << "Array a:\n";
        for (size_t i = 0; i < a.size(); ++i)
            std::cout << "a[" << i << "] = " << a[i] << '\n';

        Array<int> b = a;
        b[0] = 999;

        std::cout << "\nArray b (copy of a, modified b[0]):\n";
        for (size_t i = 0; i < b.size(); ++i)
            std::cout << "b[" << i << "] = " << b[i] << '\n';

        std::cout << "\nArray a (should be unchanged):\n";
        for (size_t i = 0; i < a.size(); ++i)
            std::cout << "a[" << i << "] = " << a[i] << '\n';

        std::cout << "\nAccessing out-of-bounds index...\n";
        std::cout << a[10] << '\n'; // should throw

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << '\n';
    }

    return 0;
}

