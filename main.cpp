#include <iostream>

#include "Regular.hh"

int main() {
    using namespace regular_string;

    std::string s = "Hello, world!";
    auto m = rk(ru({rs('H'), rs('e')->label("fuck"), rs('l')}))->match(s.cbegin(), s.cend());
    std::cout << m->success << '\n';
    if (m->success) {
        std::cout << s.substr(0, std::size_t(m->end - s.cbegin())) << '\n';
    }
    return 0;
}