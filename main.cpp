#include <iostream>

#include "include.hh"

int main() {
    auto p = std::make_shared<reg::pattern::EmptyString<char>>();
    std::string s;
    p->match(s.cbegin(), s.cend());
    return 0;
}
