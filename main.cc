#include <iostream>
#include <fstream>
#include <locale>
#include <codecvt>
#include <string>
#include <sstream>

#include "regular.h"
#include "regular.cc"


int main(int argc, char *argv[]) {
    using namespace regular::shortcut::wide;

    /**
     * 手动预处理
     *      补齐标签花括号
     *      去除“\n\n”
     */
    // 手动完成

    /**
     * 读取原始数据
     */
    std::ifstream ifs(argv[1]);
    std::stringstream ss;
    ss << ifs.rdbuf();
    ifs.close();
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
    auto source = convert.from_bytes(ss.str());

    /**
     * 提取
     */
    auto p_whitespace = wplc(L"\f\n\r\t\v ");
    auto p_whitespaces = wplc({p_whitespace, wpk(p_whitespace)});
    auto p_digit = wpsu({wpsr(L'0', L'9'), wpsr(L'A', L'F'), wpss(L".e")});
    auto p_number = wplc({wplu({wpss(L"+-"), wpe()}), p_digit, wpk(p_digit)});
    auto p_alphabet = wpsu({wpsr(L'a', L'z'), wpsr(L'A', L'Z')});
    auto p_word = wplc({p_alphabet, wpk(p_alphabet)});

    auto p_http = wplc({wplc(L"http://"), wplu({p_word, p_digit, wpss(L"/.")})});
    auto p = wpk(wplu(
            {
                    {L"LABEL", wplc(
                            {
                                    wplc(L"{{"),
                                    {L"MAIN", wplc(
                                            {
                                                    {L"CATEGORY", wpq(wpk(wpsd({wpsa(), wpsc(L':')})))},
                                                    wpsc(L':'),
                                                    {L"CONTENT", wpk(wplu(
                                                            {
                                                                    {L"HTTP", wpq(wplc(
                                                                            {
                                                                                    wplc(L"http://"),
                                                                                    wpk(wpsd({wpsa(), wpsc(L'}')}))
                                                                            }
                                                                    ))},
                                                                    wpq(p_whitespace),
                                                                    {L"HTTP", wpq(p_http)},
                                                                    {L"NUMBER", wpq(p_number)},
                                                                    {L"WORD", wpq(p_word)},
                                                                    {L"ATOM", wpsd({wpsa(), wpsc(L'}')})}
                                                            }
                                                    ))}
                                            }
                                    )},
                                    wplc(L"}}")
                            }
                    )},
                    wpq(p_whitespaces),
                    {L"HTTP", wpq(p_http)},
                    {L"NUMBER", wpq(p_number)},
                    {L"WORD", wpq(p_word)},
                    {L"ATOM", wpsa()}
            }
    ));
//    auto m = r->match(source.cbegin(), source.cend());
//
    /**
     * 转换
     */
    std::wstringstream target;
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
    /**
     * 写入目标数据
     */
    std::ofstream ofs(argv[2]);
    ofs << convert.to_bytes(target.str());
    ofs.close();
    return 0;
}
