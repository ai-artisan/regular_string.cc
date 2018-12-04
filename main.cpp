#include <iostream>

#include "Regular.hh"

int main() {
    using namespace regular_string;

    std::string s = "Hello, world!";
    auto m = rk(ru({rs('H')->label("shit"), rs('e')->label("fuck"), rs('l')}))->match(s.cbegin(), s.cend());
    std::cout << m->success << '\n';
    if (m->success) {
        auto um = m->derived<rkm>()->list.front()->derived<rum>();
        std::cout << um->key << '\n';
        std::cout << s.substr(0, std::size_t(um->end - s.cbegin())) << '\n';
    }
    return 0;
}
