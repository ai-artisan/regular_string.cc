#include <iostream>

#include "include.hh"

int main() {
    std::string s;
    regular::Record<char> r = {s.cbegin(), s.cend(), s.cend()};
    return 0;
}
