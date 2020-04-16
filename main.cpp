#include <iostream>

#include "include.hh"

void f() {}

int main() {
    using namespace regular::alias::narrow;

    std::string s = "asdf1234";
    auto p = pc([](const char &c) -> bool {
        return 'a' < c && c < 'z';
    });
//    auto p = std::make_shared<regular::pattern::literal_character::Closure<char, int>>(1234, [](const int &, const char &) -> bool {
//        return true;
//    });
    auto m = p->match(s.cbegin(), s.cend());
    std::cout << m.first << '\n'
              << std::string(m.second->begin, m.second->direct_end) << '\n'
              << std::string(m.second->begin, m.second->greedy_end) << '\n';

    return 0;
}
