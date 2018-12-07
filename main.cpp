#include <iostream>

#include "Regular.hh"

int main() {
    using namespace rs;

    std::string s = "Hello, world!";
    auto r = RS();
    auto m = r->match(s.cbegin(), s.cend());
    std::cout << m->success << '\n';
    std::cout << m->as<RKM>()->list.size() << '\n';
    return 0;
}
