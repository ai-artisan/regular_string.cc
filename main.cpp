#include <iostream>

#include "Regular.hh"

int main() {
    using namespace rs;

    std::string s = "Hello, world!";
    auto r = RK(RS(), RC("world"));
    auto m = r->match(s.cbegin(), s.cend());
    std::cout << m->success << '\n';
    std::cout << m->derived<RKM>()->termination->derived<RCM>()->map.size() << '\n';
//    if (m->success) {
//        auto um = m->derived<RKM>()->repeats.front()->derived<RUM>();
//        std::cout << um->key << '\n';
//        std::cout << s.substr(0, std::size_t(um->end - s.cbegin())) << '\n';
//    }
    return 0;
}
