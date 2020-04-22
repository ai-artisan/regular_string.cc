#include <iostream>

#include "include.hh"

int main() {
    using ra = regular::Alias<char>;

    auto p = ra::pk(ra::pla({{"A", ra::pbc(ra::pc_alpha)}, {"D", ra::pbc(ra::pc_digit)}}));
    std::string s;
    s.clear();
    while (std::cin >> s) {
        auto m = p->match(s.cbegin(), s.cend());
//        std::cout << m.first << '\n'
//                  << std::string(m.second->begin, m.second->direct_end) << '\n'
//                  << std::string(m.second->begin, m.second->greedy_end) << '\n';
        if (m.first) {
            auto l = m.second->query({"A", "D"});
            std::cout << l.size() << '\n';
            for (auto &&[key, item]:l) std::cout << item->begin - s.cbegin() << ' ' << key << ' ' << std::string(item->begin, item->direct_end) << '\n';
        }
    }

    return 0;
}
