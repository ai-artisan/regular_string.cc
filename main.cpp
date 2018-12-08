#include <iostream>

#include "Regular.hh"

int main() {
    using namespace rs;

//    std::shared_ptr<Regular> r;
//    r = RC()->item(RS('('))->item(RK(
//            RU()->item(r)->item(RS())
//    ))->item(RS(')'));

//    auto r = RC()->item(RC("Hello"), "A")->item(RS())->item(RS())->item(RC("world"), "B");
    auto r = RC()->item(RC("He"))->item(RS())->item(RS())->item(RS('o'));

    std::string s = "Hello, world!";
    auto m = r->match(s.cbegin(), s.cend());
    std::cout << m->success << '\n';
//    std::cout << m->as<RCM>()->map.at("prefix")->as<RKM>()->list.size() << '\n';
    return 0;
}
