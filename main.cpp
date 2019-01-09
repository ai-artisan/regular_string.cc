#include <iostream>
#include <fstream>

#include "Regular.hh"

using Json=json::Json<>;

int main() {
    using namespace rs;

    auto r_spaces = RK(RSIS(" \t\r\f\n\v"));
    auto r_identifier_prefix = RSRL({{'a', 'z'},
                                     {'A', 'Z'},
                                     {'_', '_'}});
    auto r_digit = RSR('0', '9');
    auto r_identifier = RC()->item(r_identifier_prefix)->item(RK(
            RU()->item(r_identifier_prefix)->item(r_digit)
    ))->collapse(true);

    auto r=RK();

    std::ifstream ifs("../text");
    std::stringstream ss;
    ss << ifs.rdbuf();
    auto s = ss.str();
    auto m = r->match(s.cbegin(), s.cend());
    std::cout << m->success << '\n';
    if (m->success) std::cout << Json::stringify(m->json(), 4) << '\n';
    return 0;
}
