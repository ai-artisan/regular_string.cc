#include <iostream>

#include "include.hh"

int main() {
    using ra = regular::Alias<char>;

    auto p = ra::plc(ra::pc_alnum, 4);
    std::string s;
    s.clear();
    while (std::cin >> s) {
        auto m = p->match(s.cbegin(), s.cend());
        std::cout << m.first << '\n'
                  << std::string(m.second->begin, m.second->direct_end) << '\n'
                  << std::string(m.second->begin, m.second->greedy_end) << '\n';
    }

    return 0;
}
