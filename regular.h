#pragma once

#include <functional>
#include <memory>
#include <string>

#include "json/json.hh"

namespace regular {
    template<typename>
    struct Traits {
        virtual ~Traits() = 0;
    };

    template<>
    struct Traits<char> {
        virtual ~Traits() = 0;

        using Character=char;
        using String=std::string;
        using Json=json::Json<double, char>;
    };

    template<>
    struct Traits<wchar_t> {
        virtual ~Traits() = 0;

        using Character=wchar_t;
        using String=std::wstring;
        using Json=json::Json<double, wchar_t>;
    };

    template<typename CharacterType>
    struct Pattern : Traits<CharacterType>, std::enable_shared_from_this<Pattern<CharacterType>> {
        struct Record : std::enable_shared_from_this<Record> {
            const bool success;
            const typename Pattern::String::const_iterator begin, end;

            Record(const bool &, const typename Pattern::String::const_iterator &, const typename Pattern::String::const_iterator &);

            virtual typename Pattern::Json json();

            template<typename Derived>
            std::shared_ptr<Derived> as();
        };

        virtual std::shared_ptr<Record> match(const typename Pattern::String::const_iterator &, const typename Pattern::String::const_iterator &) = 0;
    };

    namespace pattern {
        template<typename CharacterType>
        struct Null : Pattern<CharacterType> {
            std::shared_ptr<typename Null::Record> match(const typename Null::String::const_iterator &, const typename Null::String::const_iterator &) final;
        };

        template<typename CharacterType, typename Context=nullptr_t>
        struct Singleton : Pattern<CharacterType> {
            std::shared_ptr<typename Singleton::Record> match(const typename Singleton::String::const_iterator &, const typename Singleton::String::const_iterator &) final;

            const std::function<bool(const CharacterType &, const Context &)> describe;
            const Context context;

            explicit Singleton(decltype(describe) &&, Context && = nullptr);
        };

        template<typename CharacterType>
        struct Linear : Pattern<CharacterType> {
            virtual ~Linear() = 0;

            const std::list<std::pair<
                    typename Linear::String,
                    std::shared_ptr<Pattern<CharacterType>>
            >> sequence;

            Linear(decltype(sequence) &&);
        };

        namespace linear {


        }
    }
}