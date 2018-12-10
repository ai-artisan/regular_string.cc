#include <iostream>

#include "Regular.hh"

int main() {
    using namespace rs;

    auto ru = RU();
    auto r = RC()->item(RS('('))->item(RK(ru))->item(RS(')'));
    ru->item(r)->item(RS());

    std::string s = "()";
    auto m = r->match(s.cbegin(), s.cend());
    std::cout << m->success << '\n';
    std::cout << m->end - s.cbegin() << '\n';
    return 0;
}
