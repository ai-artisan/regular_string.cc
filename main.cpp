#include <iostream>

#include "Regular.hh"

int main() {
    using namespace rs;

    std::string s = "Hello, world!";
    auto r = RC()->item(RK(RS()), "prefix")->item(RS(','));
    auto m = r->match(s.cbegin(), s.cend());
    std::cout << m->success << '\n';
    std::cout << m->as<RCM>()->map.at("prefix")->as<RKM>()->list.size() << '\n';
    return 0;
}
