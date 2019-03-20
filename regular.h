#pragma once

#include <memory>
#include <string>

#include "json/json.hh"

namespace regular {
    template<typename>
    struct Traits {
        ~Traits() = delete;
    };

    template<>
    struct Traits<char> {
        ~Traits() = delete;

        using String=std::string;
        using Json=json::Json<double, char>;
    };

    template<>
    struct Traits<wchar_t> {
        ~Traits() = delete;

        using String=std::wstring;
        using Json=json::Json<double, wchar_t>;
    };

    template<typename Char>
    class Pattern : std::enable_shared_from_this<Pattern<Char>> {
    public:
        using String=typename Traits<Char>::String;
        using Json=typename Traits<Char>::Json;

        struct Record : std::enable_shared_from_this<Record> {
            const bool success = false;
            const typename String::const_iterator begin, end;

            template<typename Derived>
            inline std::shared_ptr<Derived> as() {
                return std::static_pointer_cast<Derived>(this->shared_from_this());
            }

            inline virtual Json json() {
                return String(begin, end);
            }
        };

        virtual std::shared_ptr<Record> match(
                const typename String::const_iterator &,
                const typename String::const_iterator &
        ) = 0;
    };
}