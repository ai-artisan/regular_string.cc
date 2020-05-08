#pragma once

#include "regular.h"

namespace regular {
    template<typename Character>
    struct Alias {
        ~Alias() = delete;

        using ct = CharacterTraits<Character>;

        using rt = Record<Character>;

        using pt = Pattern<Character>;
        using pot = pattern::EmptyString<Character>;
        using pct = pattern::LiteralCharacter<Character>;
        template<typename Context>
        using pcct = pattern::literal_character::Closure<Character, Context>;
        using pbt = pattern::Binary<Character>;
        using pbat = pattern::binary::Alternation<Character>;
        using pbct = pattern::binary::Concatenation<Character>;
//        using plt = pattern::Linear<Character>;
//        using plat = pattern::linear::Alternation<Character>;
//        using plct = pattern::linear::Concatenation<Character>;
//        using put = pattern::Unary<Character>;
//        using pkt = pattern::unary::KleeneStar<Character>;
//        using ppt = pattern::Placeholder<Character>;
//        using pmt = pattern::unary::Mark<Character>;
//
        static inline auto po = std::make_shared<pot>();

        static auto pc(const typename pct::Describe &describe) { return std::make_shared<pct>(describe); }

        template<typename Context>
        static auto pc(Context context, const typename pcct<Context>::Depict &depict) {
            return std::make_shared<pcct<Context>>(std::move(context), depict);
        }

        static inline auto pca = pc([](const Character &) -> bool { return true; });

        static auto pc(const Character &c0) {
            return pc(c0, [](const Character &c0, const Character &c) -> bool { return c == c0; });
        }

        static auto pc(const typename ct::String &s) {
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

        static inline auto pc_ascii = pc(0x00, 0x7F);

        static inline auto pc_lower = pc(ct::character('a'), ct::character('z'));

        static inline auto pc_upper = pc(ct::character('A'), ct::character('Z'));

        static inline auto pc_alpha = pc({pc_upper, pc_lower});

        static inline auto pc_digit = pc(ct::character('0'), ct::character('9'));

        static inline auto pc_alnum = pc({pc_alpha, pc_digit});

        static inline auto pc_blank = pc(ct::string(" \t"));

        static inline auto pc_space = pc(ct::string(" \t\r\n\v\f"));

        static inline auto pc_cntrl = pc({pc(0x00, 0x1F), pc(0x7F)});

        static inline auto pc_graph = pc(0x21, 0x7E);

        static inline auto pc_print = pc(0x20, 0x7E);

        static inline auto pc_punct = pc(ct::string("[]!\"#$%&'()*+,./:;<=>?@\\^_`{|}~-"));

        static inline auto pc_xdigit = pc(
                {
                        pc(ct::character('A'), ct::character('F')),
                        pc(ct::character('a'), ct::character('f')),
                        pc(ct::character('0'), ct::character('9'))
                }
        );

        static auto pba(const std::shared_ptr<pt> &first, const std::shared_ptr<pt> &second) {
            return std::make_shared<pbat>(first, second);
        }

        static auto pba(const std::shared_ptr<pt> &target) { return pba(target, po); }

        static auto pbc(const std::shared_ptr<pt> &first, const std::shared_ptr<pt> &second) {
            return std::make_shared<pbct>(first, second);
        }
//
//        static auto pla(typename plt::List list) {
//            return std::make_shared<plat>(std::move(list));
//        }
//
//        static auto plc(typename plt::List list) {
//            return std::make_shared<plct>(std::move(list));
//        }
//
//        static auto plc(const typename ct::String &s) {
//            typename plt::List list;
//            for (auto &&c:s) list.emplace_back(pc(c));
//            return std::make_shared<plct>(std::move(list));
//        }
//
//        static auto plc(const std::shared_ptr<pt> &item, const std::size_t &nt) {
//            return plc(typename plt::List(nt, item));
//        }
//
//        static auto pk(const std::shared_ptr<pt> &value) {
//            return std::make_shared<pkt>(value);
//        }
//
//        static auto pbc(const std::shared_ptr<pt> &target) {
//            return pbc(target, pk(target));
//        }
//
//        static auto pp() { return std::make_shared<ppt>(); }
//
//        static auto pm(typename ct::String tag, const std::shared_ptr<pt> &value) {
//            return std::make_shared<pmt>(value, std::move(tag));
//        }
//
//        static auto pm(const std::shared_ptr<pt> &value) {
//            return pm(L"", value);
//        }
    };
}