#include <iostream>
#include <fstream>

#include "Regular.hh"

using Json=json::Json<>;

int main() {
    using namespace rs;

    auto ru = RU();
    auto r = RC()->item(RS('('))->item(RK(ru), "content")->item(RS(')'));
    ru->item(r)->item(RK(RS("()", false))->merge(true));
//    ru->item(r)->item(
//            RI()->item(RK(RS("()", false)))->item(
//                    RC()->item(RS())->item(RK(RS()))
//            )->merge(true)
//    );

    std::ifstream ifs("../text");
    std::stringstream ss;
    ss << ifs.rdbuf();
    auto s = ss.str();
    auto m = r->match(s.cbegin(), s.cend());
    std::cout << m->success << '\n';
    if (m->success) std::cout << Json::stringify(m->json(), 4) << '\n';
    return 0;
}
