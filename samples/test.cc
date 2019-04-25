#include "../regular.hh"

#include <iostream>

int main() {
    using namespace regular::shortcut::narrow;

    auto p0 = plcs("asdf");

    auto p = pk(pld({plu({p0, psa}), p0}));

    auto[b, r]=p->as<pt>()->match("1234asasdf");

    std::cout << b << '\n'
              << r->string() << '\n';

    return 0;
}