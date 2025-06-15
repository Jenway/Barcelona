#include "Harl.hpp"
#include <iostream>

int get_level_index(const std::string &level) {
    const std::string levels[] = {"DEBUG", "INFO", "WARNING", "ERROR"};
    for (int i = 0; i < 4; ++i) {
        if (levels[i] == level)
            return i;
    }
    return -1;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: ./harlFilter <LEVEL>\n";
        return 1;
    }

    Harl harl;
    int level_index = get_level_index(argv[1]);

    switch (level_index) {
#if __cplusplus > 201103L
        case 0:
            harl.complain("DEBUG");
            [[fallthrough]];
        case 1:
            harl.complain("INFO");
            [[fallthrough]];
        case 2:
            harl.complain("WARNING");
            [[fallthrough]];
        case 3:
            harl.complain("ERROR");
            break;
#else
        case 0:
            harl.complain("DEBUG");
            // fall through
        case 1:
            harl.complain("INFO");
            // fall through
        case 2:
            harl.complain("WARNING");
            // fall through
        case 3:
            harl.complain("ERROR");
            break;
#endif

        default:
            std::cout << "[ Probably complaining about insignificant problems ]\n";
    }

    return 0;
}
