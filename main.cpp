#include <iostream>

#include "include.hh"

inline std::string string(std::shared_ptr<regular::Record<char>> r) {
    return std::string(r->begin, r->direct_end);
}

int main() {
    using ra = regular::Alias<char>;

    auto p = ra::pbc(ra::pba(
            ra::pm("A", ra::pbc(ra::pc_alpha)),
            ra::pm("D", ra::pbc(ra::pc_digit))
    ));
    std::string s;
    s.clear();
    while (std::cin >> s) {
        auto[b, r] = p->match(s.cbegin(), s.cend());
        std::cout << b << '\n';
//                  << std::string(r->begin, r->direct_end) << '\n'
//                  << std::string(r->begin, r->greedy_end) << '\n';
        if (b) {
            auto l = r->extractList();
            std::cout << l.size() << '\n';
            for (auto &&[tag, item]:l) std::cout << tag << ' ' << string(item) << '\n';
            auto m = r->extractDict();
            std::cout << m.size() << '\n';
            if (m.find("A") != m.cend()) std::cout << "A: " << string(m.at("A")) << '\n';
            if (m.find("D") != m.cend()) std::cout << "D: " << string(m.at("D")) << '\n';
        }
    }

    return 0;
}
