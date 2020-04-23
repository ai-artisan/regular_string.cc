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

namespace regular {
    template<typename...>
    struct CharacterTraits;

    template<typename>
    struct Data;

    namespace data {
        template<typename>
        struct List;

        template<typename>
        struct Dict;
    }

    template<typename>
    struct Record;

    namespace record {
        template<typename>
        struct Some;

        template<typename>
        struct LinearSome;

        template<typename>
        struct Every;

        template<typename>
        struct LinearEvery;

        template<typename>
        struct Greedy;

        template<typename>
        struct Mark;

        namespace mark {
            template<typename>
            struct List;

            template<typename>
            struct Dict;
        }
    }

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

            template<typename>
            struct Filter;
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
            struct Collapse;

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