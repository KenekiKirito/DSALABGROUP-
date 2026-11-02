#pragma once

#include <cctype>
#include <string>

namespace jc8 {

inline std::string trim(const std::string& s) {
    std::size_t b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) {
        return "";
    }
    std::size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

inline std::string toUpper(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return s;
}

inline std::string toLower(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

}
