#pragma once

#include "regular.h"

namespace regular {
    template<typename Character>
    struct Alias {
        ~Alias() = delete;

        using rt = regular::Record<Character>;
        using rut = regular::record::Unary<Character>;
        using rst = regular::record::Some<Character>;
        using rbt = regular::record::Binary<Character>;
        using ret = regular::record::Every<Character>;
        using rgt = regular::record::Greedy<Character>;

        using pt = regular::Pattern<Character>;
        using pot = regular::pattern::EmptyString<Character>;
        using pct = regular::pattern::LiteralCharacter<Character>;
        template<typename Context>
        using pcct = regular::pattern::literal_character::Closure<Character, Context>;

        static inline auto po() { return std::make_shared<pot>(); }

        static inline auto pc(
                const typename pattern::LiteralCharacter<Character>::Describe &describe
        ) { return std::make_shared<pct>(describe); }

        template<typename Context>
        static inline auto pc(
                Context context,
                const typename pattern::literal_character::Closure<Character, Context>::Depict &depict
        ) { return std::make_shared<pcct<Context>>(std::move(context), depict); }

        static inline auto pc() { return pc([](const Character &) -> bool { return true; }); }

        static inline auto pc(const Character &c0) {
            return pc(c0, [](const Character &c0, const Character &c) -> bool { return c == c0; });
        }

        static auto pc(const typename CharacterTraits<Character>::String &s) {
            using Map = std::unordered_map<Character, std::nullptr_t>;
            Map map;
            for (auto &&c:s) map[c] = nullptr;
            return pc(std::move(map), [](const Map &map, const Character &c) -> bool {
                return map.find(c) != map.end();
            });
        }

        static auto pc(const Character &inf, const Character &sup) {
            using Array = std::array<Character, 2>;
            return pc(Array{inf, sup}, [](const Array &array, const Character &c) -> bool {
                return array[0] <= c && c <= array[1];
            });
        }

        static auto pc(std::list<std::shared_ptr<pct>> list) {
            return pc(std::move(list), [](const decltype(list) &list, const Character &c) -> bool {
                for (auto &&p:list) if (p->describe(c)) return true;
                return false;
            });
        }

        static auto pc(const std::shared_ptr<pct> &first, const bool &sign, const std::shared_ptr<pct> &second) {
            using Tuple = std::tuple<std::shared_ptr<pct>, bool, std::shared_ptr<pct>>;
            return pc(Tuple{first, sign, second}, [](const Tuple &tuple, const Character &c) -> bool {
                auto &&[first, sign, second]=tuple;
                if (first->describe(c)) return !(sign ^ second->describe(c));
                else return false;
            });
        }
    };
}