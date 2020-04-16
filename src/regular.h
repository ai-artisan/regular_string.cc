#pragma once

#include <array>
#include <codecvt>
#include <functional>
#include <list>
#include <locale>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace regular {
    template<typename T>
    using Ptr=std::shared_ptr<T>;

    template<typename...>
    struct CharacterTraits;

    template<>
    struct CharacterTraits<char>;

    template<>
    struct CharacterTraits<wchar_t>;

    template<typename>
    struct Record;

    namespace record {
        template<typename>
        struct Unary;

        template<typename>
        struct Some;

        template<typename>
        struct Binary;

        template<typename>
        struct Every;

        template<typename>
        struct Greedy;
    }

    template<typename>
    struct Pattern;

    namespace pattern {
        template<typename>
        struct EmptyString;

        template<typename>
        struct LiteralCharacter;

        namespace literal_character {
            template<typename>
            struct Closure;
        }

        template<typename>
        struct Alternation;

        template<typename>
        struct Concatenation;

        template<typename>
        struct Linear;

        namespace linear {
            template<typename>
            struct Alternation;

            template<typename>
            struct Concatenation;
        }

        template<typename>
        struct KleeneStar;

        template<typename>
        struct Filter;

        template<typename>
        struct Placeholder;

        template<typename>
        struct Collapse;
    }
}

#include "CharacterTraits.h"
#include "Record.h"