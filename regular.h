#pragma once

#include <functional>
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

    template<typename Character=char>
    struct Pattern : std::enable_shared_from_this<Pattern<Character>> {
        struct Record : std::enable_shared_from_this<Record> {
            const bool success;
            const typename Traits<Character>::String::const_iterator begin, end;

            Record(const bool &, const typename Traits<Character>::String::const_iterator &, const typename Traits<Character>::String::const_iterator &);

            virtual typename Traits<Character>::Json json();

            template<typename Derived>
            std::shared_ptr<Derived> as();
        };

        virtual std::shared_ptr<Record> match(const typename Traits<Character>::String::const_iterator &, const typename Traits<Character>::String::const_iterator &) = 0;
    };

    namespace pattern {
        template<typename Character>
        struct Null : Pattern<Character> {
            using Record=typename Null::Record;

            std::shared_ptr<Record> match(const typename Traits<Character>::String::const_iterator &, const typename Traits<Character>::String::const_iterator &) final;
        };

        template<typename Character, typename Context=nullptr_t>
        struct Singleton : Pattern<Character> {
            using Record=typename Singleton::Record;

            std::shared_ptr<Record> match(const typename Traits<Character>::String::const_iterator &, const typename Traits<Character>::String::const_iterator &) final;

            const std::function<bool(const Character &, const Context &)> describe;
            const Context context;

            explicit Singleton(const decltype(describe) &, const Context & = nullptr);
        };
    }
}