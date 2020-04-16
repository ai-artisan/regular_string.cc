#include <iostream>

#include "include.hh"

int main() {
    using ra = regular::Alias<char>;

//    auto p = ra::po();
//    auto p = ra::pc([](const char &c) -> bool {
//        return 'a' <= c && c <= 'z';
//    });
//    auto p = ra::pc('a', [](const char &context, const char &c) -> bool {
//        return c == context;
//    });
//    auto p = ra::pc();
//    auto p = ra::pc('a');
//    auto p = ra::pc("asdf1234");
//    auto p = ra::pc('a', 'z');
//    auto p = ra::pc({ra::pc('a'), ra::pc('1')});
    auto p = ra::pc(ra::pc('a', 'z'), false, ra::pc('a', 'g'));
    std::string s;
    s.clear();
    while (std::cin >> s) {
        auto m = p->match(s.cbegin(), s.cend());
        std::cout << m.first << '\n'
                  << std::string(m.second->begin, m.second->direct_end) << '\n'
                  << std::string(m.second->begin, m.second->greedy_end) << '\n';
    };

    return 0;
}
