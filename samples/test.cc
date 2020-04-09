#include "../include.hh"

#include <iostream>

int main() {
//    using namespace reg::shortcut::narrow;
//
//    auto p0 = plcs("asdf");
//
//    auto p = pk(pld({psa, p0}));
//
//    auto[b, r]=p->as<pt>()->match("1234asasdf");
//
//    std::cout << b << '\n'
//              << r->string() << '\n';

    using namespace reg::shortcut::wide;
    std::wstring s = L"null";
    auto[b, r]=wplcs(L"null")->as<wpt>()->match(s);
    std::wcout << b << L'\n'
               << r->string().at(0) << L'\n';

    return 0;
}