#include <iostream>

#include "include.hh"

int main() {
    using ra = regular::Alias<char>;

    std::string s;
    while (std::cin >> s) {
        auto[success, begin, end, children] = *ra::pbc(ra::pc('a'), ra::pc('1'))->match(s);
        std::cout << success << '\n'
                  << std::string(begin, end) << '\n';
    }

    return 0;
}
