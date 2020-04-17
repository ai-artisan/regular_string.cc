#pragma once

#include "regular.h"

namespace regular {
    template<typename Character>
    struct Alias {
        ~Alias() = delete;

        using rt = Record<Character>;
        using rbst = record::Some<Character>;
        using rlst = record::LinearSome<Character>;
        using rbet = record::Every<Character>;
        using rlet = record::LinearEvery<Character>;
        using rgt = record::Greedy<Character>;

        using pt = Pattern<Character>;
        using pot = pattern::EmptyString<Character>;
        using pct = pattern::LiteralCharacter<Character>;
        template<typename Context>
        using pcct = pattern::literal_character::Closure<Character, Context>;
        using pbt = pattern::Binary<Character>;
        using pbat = pattern::binary::Alternation<Character>;
        using pbct = pattern::binary::Concatenation<Character>;
        using plt = pattern::Linear<Character>;
        using plat = pattern::linear::Alternation<Character>;
        using plct = pattern::linear::Concatenation<Character>;

        static inline auto po() { return std::make_shared<pot>(); }

        static inline auto pc(const typename pct::Describe &describe) { return std::make_shared<pct>(describe); }

        template<typename Context>
        static inline auto pc(Context context, const typename pcct<Context>::Depict &depict) {
            return std::make_shared<pcct<Context>>(std::move(context), depict);
        }

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

        static inline auto pba(const std::shared_ptr<pt> &first, const std::shared_ptr<pt> &second) {
            return std::make_shared<pbat>(first, second);
        }

        static inline auto pba_zo(const std::shared_ptr<pt> &p) { return pba(p, po()); }

        static inline auto pbc(const std::shared_ptr<pt> &first, const std::shared_ptr<pt> &second) {
            return std::make_shared<pbct>(first, second);
        }

        static inline auto pla(typename plt::List list) {
            return std::make_shared<plat>(std::move(list));
        }
    };
}