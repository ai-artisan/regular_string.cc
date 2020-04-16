#pragma once

#include "reg.h"

#define CHARACTER char

#ifdef CHARACTER

using rt=reg::Record<CHARACTER>;
using rbt=reg::record::Binary<CHARACTER>;
using rst=reg::record::Some<CHARACTER>;
using ret=reg::record::Every<CHARACTER>;
using rgt=reg::record::Greedy<CHARACTER>;

using pt=reg::Pattern<CHARACTER>;
using pot=reg::pattern::EmptyString<CHARACTER>;
using pct=reg::pattern::LiteralCharacter<CHARACTER>;
template<typename Context>
using pcct=reg::pattern::literal_character::Closure<CHARACTER, Context>;
using plt=reg::pattern::Linear<CHARACTER>;
using plat=reg::pattern::linear::Alternation<CHARACTER>;
using plct=reg::pattern::linear::Concatenation<CHARACTER>;
using pkt=reg::pattern::KleeneStar<CHARACTER>;
using p2t=reg::pattern::Operation<CHARACTER>;
using ppt=reg::pattern::Placeholder<CHARACTER>;
using pqt=reg::pattern::Collapsed<CHARACTER>;

inline std::shared_ptr<pot> po() {
    return std::make_shared<pot>();
}

inline std::shared_ptr<pct> pc(std::function<bool(const CHARACTER &)> describe) {
    return std::make_shared<pct>(std::move(describe));
}

template<typename Context>
inline std::shared_ptr<pcct<Context>> pc(
        Context context,
        std::function<bool(const Context &, const CHARACTER &)> depict
) {
    return std::make_shared<pcct<Context>>(std::move(context), std::move(depict));
}

inline std::shared_ptr<pct> pc() {
    return std::make_shared<pct>([](const CHARACTER &) {
        return true;
    });
}

inline std::shared_ptr<pcct<CHARACTER>> pc(const CHARACTER &c0) {
    return std::make_shared<pcct<CHARACTER>>(
            CHARACTER(c0),
            [](const CHARACTER &c0, const CHARACTER &c) -> bool {
                return c == c0;
            }
    );
}

std::shared_ptr<pcct<std::unordered_map<
        CHARACTER,
        std::nullptr_t
>>> pc(const typename reg::Traits<CHARACTER>::String &s) {
    std::unordered_map<CHARACTER, std::nullptr_t> m;
    for (char i : s) m[i] = nullptr;
    return std::make_shared<pcct<std::unordered_map<
            CHARACTER,
            std::nullptr_t
    >>>(std::move(m), [&](const std::unordered_map<
            CHARACTER,
            std::nullptr_t
    > &m, const CHARACTER &c) -> bool {
        return m.find(c) != m.cend();
    });
}

inline std::shared_ptr<pcct<std::array<CHARACTER, 2>>> pc(const CHARACTER &inf, const CHARACTER &sup) {
    return std::make_shared<pcct<std::array<CHARACTER, 2>>>(
            std::array{inf, sup},
            [&](const std::array<CHARACTER, 2> &interval, const CHARACTER &c) -> bool {
                return interval[0] <= c && c <= interval[1];
            }
    );
}

std::shared_ptr<pcct<std::vector<std::shared_ptr<pct>>>> pca(std::vector<std::shared_ptr<pct>> vector) {
    return std::make_shared<pcct<std::vector<std::shared_ptr<pct>>>>(
            std::move(vector),
            [&](const std::vector<std::shared_ptr<pct>> &vector, const CHARACTER &c) -> bool {
                for (const auto &i : vector) if (i->describe(c)) return true;
                return false;
            }
    );
}

std::shared_ptr<pcct<std::tuple<
        std::shared_ptr<pct>, bool, std::shared_ptr<pct>
>>> pc2(std::shared_ptr<pct> first, const bool &sign, std::shared_ptr<pct> second) {
    return std::make_shared<pcct<std::tuple<
            std::shared_ptr<pct>, bool, std::shared_ptr<pct>
    >>>(
            std::tuple{std::move(first), sign, std::move(second)},
            [&](const std::tuple<
                    std::shared_ptr<pct>, bool, std::shared_ptr<pct>
            > &tuple, const CHARACTER &c) -> bool {
                auto b = std::get<0>(tuple)->describe(c);
                if (b) return !(std::get<1>(tuple) ^ (b && std::get<2>(tuple)->describe(c)));
                else return false;
            }
    );
}

inline std::shared_ptr<plat> pla(std::vector<typename plt::Item> vector) {
    return std::make_shared<plat>(std::move(vector));
}

inline std::shared_ptr<plct> plc(std::vector<typename plt::Item> vector) {
    return std::make_shared<plct>(std::move(vector));
}

std::shared_ptr<plct> plcs(const typename reg::Traits<CHARACTER>::String &s) {
    std::vector<typename plt::Item> linear(s.size(), nullptr);
    for (auto i = s.cbegin(); i != s.cend(); i++) linear[i - s.cbegin()] = pc(*i);
    return std::make_shared<plct>(std::move(linear));
}

inline std::shared_ptr<pkt> pk(std::shared_ptr<pt> item) {
    return std::make_shared<pkt>(std::move(item));
}

inline std::shared_ptr<p2t> p2(std::shared_ptr<pt> first, const bool &sign, std::shared_ptr<pt> second) {
    return std::make_shared<p2t>(sign, p2t::Array{std::move(first), std::move(second)});
}

inline std::shared_ptr<ppt> pp() {
    return std::make_shared<ppt>();
}

inline std::shared_ptr<pqt> pq(std::shared_ptr<pt> p) {
    return std::make_shared<pqt>(std::move(p));
}

inline std::shared_ptr<plct> plc_om(const std::shared_ptr<pt>& p) {
    return plc({{reg::Traits<CHARACTER>::string("First"), p},
                {reg::Traits<CHARACTER>::string("Rest"),  pk(p)}});
}

//static inline std::shared_ptr<plat> pla_oz(const std::shared_ptr<pt> &p) {
//    return hub::plu({{Traits<CHARACTER>::string("One"),  p},
//                     {Traits<CHARACTER>::string("Zero"), hub::po()}});
//}
//
//static inline std::shared_ptr<typename hub::pst> ps_dgt() {
//    return hub::ps(48, 57);
//}
//
//static inline std::shared_ptr<typename hub::pst> ps_abdu() {
//    return hub::psu({hub::ps(97, 122), hub::ps(65, 90), ps_dgt(), hub::ps(95)});
//}
//
//static inline std::shared_ptr<typename hub::pst> ps_ws() {
//    return hub::ps(Traits<Character>::string(" \f\n\r\t\v"));
//}
//
//static std::list<typename Traits<Character>::String> split(const typename Traits<Character>::String &s, const std::shared_ptr<typename hub::pt> &p_sep) {
//    std::list<typename Traits<Character>::String> l;
//
//    std::shared_ptr<typename hub::pt>
//            p_seg = hub::pq(hub::pk(hub::pld({hub::psa(), p_sep}))),
//            p = hub::plc({p_seg, hub::pk(hub::plc({hub::pq(p_sep), p_seg}))});
//
//    auto r = p->match(s.cbegin(), s.cend()).record->template as<typename hub::rlet>();
//    auto &rv0 = r->vector[0];
//    l.emplace_back(typename Traits<Character>::String(rv0->begin, rv0->direct_end));
//    auto r_rest = r->vector[1]->template as<typename hub::rgt>();
//    for (auto i = r_rest->list.cbegin(); i != r_rest->list.cend(); i++) {
//        auto &rv1 = (*i)->template as<typename hub::rlet>()->vector[1];
//        l.emplace_back(typename Traits<Character>::String(rv1->begin, rv1->direct_end));
//    }
//
//    return l;
//}

#endif
