#ifndef REPLACER_HPP
#define REPLACER_HPP

#include <string>

class Replacer {
public:
    static bool replaceFile(const std::string& filename, const std::string& s1, const std::string& s2);
};

#endif
