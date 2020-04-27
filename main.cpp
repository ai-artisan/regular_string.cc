#include <iostream>

#include "include.hh"

inline std::string string(std::shared_ptr<regular::Record<char>> r) {
    return std::string(r->begin, r->direct_end);
}

int main() {
    using ra = regular::Alias<char>;

    auto p = ra::pk(ra::pba(
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
            auto l = ra::rt::extract(r);
//            std::cout << l.size() << '\n';
//            for (auto &&[tag, item]:l) std::cout << tag << ' ' << string(item) << '\n';

            auto m = ra::rt::reduce(l);
            for (auto &&[key, items]:m) {
                std::cout << key << ' ' << items.size() << '\n';
                for (auto &&item:items) std::cout << '\t' << string(item) << '\n';
            }
        }
    }

    return 0;
}
