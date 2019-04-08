#include <iostream>
#include <fstream>
#include <locale>
#include <codecvt>
#include <string>

#include "regular.h"
#include "regular.cc"


int main(int argc, char *argv[]) {
    using namespace regular::shortcut;

    std::string s;

    std::shared_ptr<regular::Pattern<char>> p;

    s = "1sdf1234QWER==--==";
    p = psd({psr('0', '9'), psr('0', '4'), psr('0', '2')});
//    p = psd({psu({psr('0', '9'), psr('a', 'z')}), pss("asfd1234"), psu({psc('a'), psc('1')})});
//    p = pk(pu({{"digit",  psr('0', '9')},
//               {"letter", psr('a', 'z')},
//               {"LETTER", psr('A', 'Z')}}));
    auto m = p->match(s.cbegin(), s.cend());
    std::cout << m.success << "\n";
    std::cout << std::string(s.cbegin(), m.record->end) << "\n";

//    auto l = m.record->as<rk>()->list;
//    for (auto i = l.cbegin(); i != l.cend(); ({
//        std::cout << (*i)->as<rls>()->key << '\n';
//        i++;
//    }));

//    /**
//     * 手动预处理
//     *      补齐标签花括号
//     *      去除“\n\n”
//     */
//    // 手动完成
//
//    /**
//     * 读取原始数据
//     */
//    std::ifstream ifs(argv[1]);
//    std::stringstream ss;
//    ss << ifs.rdbuf();
//    ifs.close();
//    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
//    auto source = convert.from_bytes(ss.str());
//
//    /**
//     * 提取
//     */
//    auto r_whitespace = RSI(L"\f\n\r\t\v ");
//    auto r_whitespaces = RC()->item(r_whitespace)->item(RK(r_whitespace));
//    auto r_digit = RU()
//            ->item(RSR(L'0', L'9'))
//            ->item(RSR(L'A', L'F'))
//            ->item(RSI(L".e"));
//    auto r_number = RC()
//            ->item(RU()
//                           ->item(RSI(L"+-"))
//                           ->item(RN())
//            )
//            ->item(r_digit)
//            ->item(RK(r_digit));
//    auto r_alphabet = RU()
//            ->item(RSR(L'a', L'z'))
//            ->item(RSR(L'A', L'Z'));
//    auto r_word = RC()
//            ->item(r_alphabet)
//            ->item(RK(r_alphabet));
//    auto r_http = RC()
//            ->item(RC(L"http://"))
//            ->item(RU()
//                           ->item(r_word)
//                           ->item(r_digit)
//                           ->item(RSI(L"/."))
//            );
//    auto r = RK(RU()
//                        ->item(RC()
//                                       ->item(RC(L"{{"))
//                                       ->item(RC()
//                                                      ->item(CLPSD(RK(RSI(L':', false))), L"CATEGORY")
//                                                      ->item(RSI(L':'))
//                                                      ->item(RK(RU()
//                                                                        ->item(CLPSD(RC()
//                                                                                             ->item(RC(L"http://"))
//                                                                                             ->item(RK(RSI(L'}', false)))
//                                                                        ), L"HTTP")
//                                                                        ->item(CLPSD(r_whitespaces))
//                                                                        ->item(CLPSD(r_http), L"HTTP")
//                                                                        ->item(CLPSD(r_number), L"NUMBER")
//                                                                        ->item(CLPSD(r_word), L"WORD")
//                                                                        ->item(RSI(L'}', false), L"ATOM")
//                                                      ), L"CONTENT"), L"MAIN"
//                                       )
//                                       ->item(RC(L"}}")), L"LABEL"
//                        )
//                        ->item(CLPSD(r_whitespaces))
//                        ->item(CLPSD(r_http), L"HTTP")
//                        ->item(CLPSD(r_number), L"NUMBER")
//                        ->item(CLPSD(r_word), L"WORD")
//                        ->item(RSA(), L"ATOM")
//    );
//    auto m = r->match(source.cbegin(), source.cend());
//
//    /**
//     * 转换
//     */
//    std::wstringstream target;
//    const auto NUMBER = L"N", WORD = L"W", HTTP = L"H";
//    auto list = m->as<RKM>()->list;
//    for (auto i = list.cbegin(); i != list.cend(); ({
//        auto m = (*i)->as<RUM>();
//        auto key = m->key;
//        auto value = m->value;
//        if (key == L"LABEL") {
//            auto m = value->as<RCM>()->map.at(L"MAIN")->as<RCM>();
//            auto category = m->map.at(L"CATEGORY")->json().asString();
//            auto list = m->map.at(L"CONTENT")->as<RKM>()->list;
//            std::list<std::wstring> cache;
//            for (auto i = list.cbegin(); i != list.cend(); ({
//                auto m = (*i)->as<RUM>();
//                auto key = m->key;
//                if (key == L"HTTP") cache.emplace_back(HTTP);
//                else if (key == L"NUMBER") cache.emplace_back(NUMBER);
//                else if (key == L"WORD") cache.emplace_back(WORD);
//                else if (key == L"ATOM") cache.emplace_back(m->value->json().asString());
//                i++;
//            }));
//            if (!cache.empty()) {
//                target << cache.front() << L"\tB-" << category << L'\n';
//                for (auto i = std::next(cache.cbegin()); i != cache.cend(); ({
//                    target << *i << L"\tI-" << category << L'\n';
//                    i++;
//                }));
//            }
//        } else if (key == L"HTTP") target << HTTP << L"\tO\n";
//        else if (key == L"NUMBER") target << NUMBER << L"\tO\n";
//        else if (key == L"WORD") target << WORD << L"\tO\n";
//        else if (key == L"ATOM") {
//            auto s = value->json().asString();
//            target << s << L"\tO\n";
//            if (RSI(L"!?。！？")->match(s.cbegin(), s.cend())->success) target << L'\n';
//        }
//        i++;
//    }));
//
//    /**
//     * 写入目标数据
//     */
//    std::ofstream ofs(argv[2]);
//    ofs << convert.to_bytes(target.str());
//    ofs.close();
    return 0;
}
