#include <iostream>

#include "include.hh"

int main() {
    using ra = regular::Alias<char>;

    std::string s;
    while (std::cin >> s) {
//        auto r = ra::pk(ra::pba(ra::pm(
//                "A", ra::pbc(ra::pc_alpha)
//        ), ra::pm(
//                "D", ra::pbc(ra::pc_digit)
//        )))->match(s);
//        auto m = ra::rt::reduce(r->children);
//        std::cout << m["A"].size() << '\n';
//        for (auto &&r:m["A"]) std::cout << '\t' << std::string(r->begin, r->end) << '\n';
//        std::cout << m["D"].size() << '\n';
//        for (auto &&r:m["D"]) std::cout << '\t' << std::string(r->begin, r->end) << '\n';

        auto l = regular::split<char>(s, ra::pc(ra::pca, false, ra::pc_alnum));
        for (auto &&i:l) std::cout << i << '\n';
    }

    return 0;
}
