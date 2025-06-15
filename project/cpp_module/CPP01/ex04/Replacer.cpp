#include "Replacer.hpp"
#include <fstream>
#include <iostream>

bool Replacer::replaceFile(const std::string& filename, const std::string& s1, const std::string& s2) {
    if (filename.empty() || s1.empty()) {
        std::cerr << "Error: filename and s1 must not be empty.\n";
        return false;
    }

    std::ifstream inFile(filename.c_str());
    if (!inFile) {
        std::cerr << "Error: Cannot open input file.\n";
        return false;
    }

    std::ofstream outFile((filename + ".replace").c_str());
    if (!outFile) {
        std::cerr << "Error: Cannot create output file.\n";
        return false;
    }

    std::string line;
    while (std::getline(inFile, line)) {
        std::string newLine;
        size_t i = 0;

        while (i < line.length()) {
            size_t pos = line.find(s1, i);
            if (pos != std::string::npos) {
                newLine.append(line.substr(i, pos - i));
                newLine.append(s2);
                i = pos + s1.length();
            } else {
                newLine.append(line.substr(i));
                break;
            }
        }

        outFile << newLine << std::endl;
    }

    return true;
}
