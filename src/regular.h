#pragma once

#include <array>
#include <codecvt>
#include <functional>
#include <list>
#include <locale>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#include "CharacterTraits.h"
#include "Record.h"

namespace regular {
    template<typename ...Types>
    std::list<Types...> operator+(const std::list<Types...> &a, const std::list<Types...> &b) {
        std::list<Types...> c;
        for (auto &&i:a) c.emplace_back(i);
        for (auto &&i:b) c.emplace_back(i);
        return c;
    }

    template<typename...>
    struct CharacterTraits;

    template<typename>
    struct Record;

    template<typename>
    struct Pattern;

    namespace pattern {
        template<typename>
        struct EmptyString;

        template<typename>
        struct LiteralCharacter;

        namespace literal_character {
            template<typename, typename>
            struct Closure;
        }

        template<typename>
        struct Binary;

        namespace binary {
            template<typename>
            struct Alternation;

            template<typename>
            struct Concatenation;
        }

        template<typename>
        struct Linear;

        namespace linear {
            template<typename>
            struct Alternation;

            template<typename>
            struct Concatenation;
        }

        template<typename>
        struct Unary;

        namespace unary {
            template<typename>
            struct KleeneStar;

            template<typename>
            struct Mark;
        }

        template<typename>
        struct Placeholder;
    }

    template<typename>
    struct Alias;
}

#include "CharacterTraits.h"
#include "Record.h"
#include "Pattern.h"
#include "Alias.h"