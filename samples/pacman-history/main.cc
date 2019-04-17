#include "../../regular.hh"

#include <functional>
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>

template<typename C>
void f(const typename regular::Traits<C>::String &name, const typename regular::Traits<C>::String &install_date) {

}

int main() {
    auto exec = [](const char *cmd) -> std::string {
        char buffer[128];
        std::string result;
        FILE *pipe = popen(cmd, "r");
        if (!pipe) throw std::runtime_error("popen() failed!");
        try {
            while (fgets(buffer, sizeof buffer, pipe) != nullptr) {
                result += buffer;
            }
        } catch (...) {
            pclose(pipe);
            throw;
        }
        pclose(pipe);
        return result;
    };

    auto s = exec("pacman -Qi");

    auto m = std::unordered_map<std::string, std::function<void()>>{
            {"en_US.UTF-8", []() {f<char>("Name", "Install Date"); }},
            {"zh_CN.UTF-8", []() { f<wchar_t>(L"名字", L"安装日期"); }}
    };
    auto i = m.find(getenv("LANG"));
    if (i != m.cend()) i->second();
}