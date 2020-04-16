#include <iostream>

#include "include.hh"

#define REGULAR_CHARACTER_TYPE char

#include "src/ALIAS.h"

#undef REGULAR_CHARACTER_TYPE

int main() {
    std::string s = "asdf1234";
    auto p = std::make_shared<regular::pattern::literal_character::Closure<char, int>>(1234, [](const int &, const char &) -> bool {
        return true;
    });
    auto r = p->match(s.cbegin(), s.cend());
    std::cout << r.first << '\n';
    return 0;
}
