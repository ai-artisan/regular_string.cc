#pragma once

#include "reg.h"

namespace reg {
    template<typename Character>
    struct hub {
        ~hub() = delete;

        using rt=Record<Character>;
        using rbt=record::Binary<Character>;
        using rst=record::Some<Character>;
        using ret=record::Every<Character>;
        using rgt=record::Greedy<Character>;

        using pt=Pattern<Character>;
        using pot=pattern::EmptyString<Character>;
        using pct=pattern::LiteralCharacter<Character>;
        template<typename Context>
        using pcct=pattern::literal_character::Closure<Character, Context>;
        using plt=pattern::Linear<Character>;
        using plat=pattern::linear::Alternation<Character>;
        using plct=pattern::linear::Concatenation<Character>;
        using pkt=pattern::KleeneStar<Character>;
        using p2t=pattern::Operation<Character>;
        using ppt=pattern::Placeholder<Character>;
        using pqt=pattern::Collapsed<Character>;

        static inline std::shared_ptr<pot> po() {
            return std::make_shared<pot>();
        }

        static inline std::shared_ptr<pct> pc(std::function<bool(const Character &)> describe) {
            return std::make_shared<pct>(std::move(describe));
        }

        template<typename Context>
        static inline std::shared_ptr<pcct<Context>> pc(
                Context context,
                std::function<bool(const Context &, const Character &)> depict
        ) {
            return std::make_shared<pcct<Context>>(std::move(context), std::move(depict));
        }

        static inline std::shared_ptr<pct> pca() {
            return std::make_shared<pct>([](const Character &) {
                return true;
            });
        }

        static inline std::shared_ptr<pcct<Character>> pc(const Character &c0) {
            return std::make_shared<pcct<Character>>(
                    Character(c0),
                    [](const Character &c0, const Character &c) -> bool {
                        return c == c0;
                    }
            );
        }

        static std::shared_ptr<pcct<std::unordered_map<
                Character,
                std::nullptr_t
        >>> pc(const typename Traits<Character>::String &s) {
            std::unordered_map<Character, std::nullptr_t> m;
            for (auto i = s.cbegin(); i != s.cend(); i++) m[*i] = nullptr;
            return std::make_shared<pcct<std::unordered_map<
                    Character,
                    std::nullptr_t
            >>>(std::move(m), [&](const std::unordered_map<
                    Character,
                    std::nullptr_t
            > &m, const Character &c) -> bool {
                return m.find(c) != m.cend();
            });
        }

        static inline std::shared_ptr<pcct<std::array<Character, 2>>> pc(const Character &inf, const Character &sup) {
            return std::make_shared<pcct<std::array<Character, 2>>>(
                    std::array{inf, sup},
                    [&](const std::array<Character, 2> &interval, const Character &c) -> bool {
                        return interval[0] <= c && c <= interval[1];
                    }
            );
        }

        static std::shared_ptr<pcct<std::vector<std::shared_ptr<pct>>>> pca(std::vector<std::shared_ptr<pct>> vector) {
            return std::make_shared<pcct<std::vector<std::shared_ptr<pct>>>>(
                    std::move(vector),
                    [&](const std::vector<std::shared_ptr<pct>> &vector, const Character &c) -> bool {
                        for (auto i = vector.cbegin(); i != vector.cend(); i++)
                            if ((*i)->describe(c)) return true;
                        return false;
                    }
            );
        }

        static std::shared_ptr<pcct<std::tuple<
                std::shared_ptr<pct>, bool, std::shared_ptr<pct>
        >>> pc2(std::shared_ptr<pct> first, const bool &sign, std::shared_ptr<pct> second) {
            return std::make_shared<pcct<std::tuple<
                    std::shared_ptr<pct>, bool, std::shared_ptr<pct>
            >>>(
                    {std::move(first), sign, std::move(second)},
                    [&](const std::tuple<
                            std::shared_ptr<pct>, bool, std::shared_ptr<pct>
                    > &tuple, const Character &c) -> bool {
                        auto b = std::get<0>(tuple)->describe(c);
                        if (b) return !(std::get<1>(tuple) ^ (b && std::get<2>(tuple)->describe(c)));
                        else return false;
                    }
            );
        }

        static inline std::shared_ptr<plat> pla(std::vector<typename plt::Item> vector) {
            return std::make_shared<plat>(std::move(vector));
        }

        static inline std::shared_ptr<plct> plc(std::vector<typename plt::Item> vector) {
            return std::make_shared<plct>(std::move(vector));
        }

        static std::shared_ptr<plct> plcs(const typename Traits<Character>::String &s) {
            std::vector<typename plt::Item> linear(s.size(), nullptr);
            for (auto i = s.cbegin(); i != s.cend(); i++) linear[i - s.cbegin()] = ps(*i);
            return std::make_shared<plct>(std::move(linear));
        }

        static inline std::shared_ptr<pkt> pk(std::shared_ptr<pt> item) {
            return std::make_shared<pkt>(std::move(item));
        }

        static inline std::shared_ptr<p2t> p2(std::shared_ptr<pt> first, const bool sign, std::shared_ptr<pt> second) {
            return std::make_shared<p2t>(sign, {first, second});
        }

        static inline std::shared_ptr<ppt> pp() {
            return std::make_shared<ppt>();
        }

        static inline std::shared_ptr<pqt> pq(const std::shared_ptr<pt> &p) {
            return std::make_shared<pqt>(p);
        }
    };

//    namespace shortcut {
//        namespace narrow {
//            using rt=hub<char>::rt;
//            using rlst=hub<char>::rlst;
//            using rlet=hub<char>::rlet;
//            using rgt=hub<char>::rgt;
//
//            using pt=hub<char>::pt;
//            using pot=hub<char>::pot;
//            using pst=hub<char>::pst;
//            template<typename Context>
//            using psct=hub<char>::psct<Context>;
//            using plt=hub<char>::plt;
//            using plut=hub<char>::plut;
//            using plit=hub<char>::plit;
//            using pldt=hub<char>::pldt;
//            using plct=hub<char>::plct;
//            using pkt=hub<char>::pkt;
//            using ppt=hub<char>::ppt;
//            using pqt=hub<char>::pqt;
//
//            const auto po = hub<char>::po();
//
//            inline std::shared_ptr<pst> ps(const std::function<bool(const char &)> &f) { return hub<char>::ps(f); }
//
//            template<typename Context>
//            inline std::shared_ptr<psct<Context>>
//            ps(Context &&c, const std::function<bool(const Context &, const char &)> &f) { return hub<char>::ps(c, f); }
//
//            const auto psa = hub<char>::psa();
//
//            inline std::shared_ptr<psct<char>> ps(const char &c) { return hub<char>::ps(c); }
//
//            inline std::shared_ptr<psct<std::unordered_map<
//                    char,
//                    std::nullptr_t
//            >>> ps(typename Traits<char>::String &&s) { return hub<char>::ps(std::move(s)); }
//
//            inline std::shared_ptr<psct<std::array<char, 2>>> ps(const char &c, const char &d) {
//                return hub<char>::ps(c, d);
//            }
//
//            inline std::shared_ptr<psct<std::vector<std::shared_ptr<pst>>>>
//            psu(std::vector<std::shared_ptr<pst>> &&l) { return hub<char>::psu(std::move(l)); }
//
//            inline std::shared_ptr<psct<std::vector<std::shared_ptr<pst>>>>
//            psi(std::vector<std::shared_ptr<pst>> &&l) { return hub<char>::psi(std::move(l)); }
//
//            inline std::shared_ptr<psct<std::pair<
//                    std::vector<std::shared_ptr<pst>>,
//                    bool
//            >>> psd(std::vector<std::shared_ptr<pst>> &&l, const bool &s = true) {
//                return hub<char>::psd(std::move(l), s);
//            }
//
//            inline std::shared_ptr<plut> plu(std::vector<typename plt::Item> &&l) {
//                return hub<char>::plu(std::move(l));
//            }
//
//            inline std::shared_ptr<plit> pli(std::vector<typename plt::Item> &&l) {
//                return hub<char>::pli(std::move(l));
//            }
//
//            inline std::shared_ptr<pldt> pld(std::vector<typename plt::Item> &&l, const bool &sign = true) {
//                return hub<char>::pld(std::move(l), sign);
//            }
//
//            inline std::shared_ptr<plct> plc(std::vector<typename plt::Item> &&l) {
//                return hub<char>::plc(std::move(l));
//            }
//
//            inline std::shared_ptr<plct> plcs(const typename Traits<char>::String &s) { return hub<char>::plcs(s); }
//
//            inline std::shared_ptr<pkt> pk(const std::shared_ptr<pt> &p) { return hub<char>::pk(p); }
//
//            inline std::shared_ptr<ppt> pp() { return hub<char>::pp(); }
//
//            inline std::shared_ptr<pqt> pq(const std::shared_ptr<pt> &p) { return hub<char>::pq(p); }
//        }
//        namespace wide {
//            using wrt=hub<wchar_t>::rt;
//            using wrlst=hub<wchar_t>::rlst;
//            using wrlet=hub<wchar_t>::rlet;
//            using wrgt=hub<wchar_t>::rgt;
//
//            using wpt=hub<wchar_t>::pt;
//            using wpot=hub<wchar_t>::pot;
//            using wpst=hub<wchar_t>::pst;
//            template<typename Context>
//            using wpsct=hub<wchar_t>::psct<Context>;
//            using wplt=hub<wchar_t>::plt;
//            using wplut=hub<wchar_t>::plut;
//            using wplit=hub<wchar_t>::plit;
//            using wpldt=hub<wchar_t>::pldt;
//            using wplct=hub<wchar_t>::plct;
//            using wpkt=hub<wchar_t>::pkt;
//            using wppt=hub<wchar_t>::ppt;
//            using wpqt=hub<wchar_t>::pqt;
//
//            const auto wpo = hub<wchar_t>::po();
//
//            inline std::shared_ptr<wpst> wps(const std::function<bool(const wchar_t &)> &f) {
//                return hub<wchar_t>::ps(f);
//            }
//
//            template<typename Context>
//            inline std::shared_ptr<wpsct<Context>>
//            wps(Context &&c, const std::function<bool(const Context &, const wchar_t &)> &f) {
//                return hub<wchar_t>::ps(c, f);
//            }
//
//            const auto wpsa = hub<wchar_t>::psa();
//
//            inline std::shared_ptr<wpsct<wchar_t>> wps(const wchar_t &c) { return hub<wchar_t>::ps(c); }
//
//            inline std::shared_ptr<wpsct<std::unordered_map<
//                    wchar_t,
//                    std::nullptr_t
//            >>> wps(typename Traits<wchar_t>::String &&s) { return hub<wchar_t>::ps(std::move(s)); }
//
//            inline std::shared_ptr<wpsct<std::array<wchar_t, 2>>>
//            wps(const wchar_t &c, const wchar_t &d) { return hub<wchar_t>::ps(c, d); }
//
//            inline std::shared_ptr<wpsct<std::vector<std::shared_ptr<wpst>>>>
//            wpsu(std::vector<std::shared_ptr<wpst>> &&l) { return hub<wchar_t>::psu(std::move(l)); }
//
//            inline std::shared_ptr<wpsct<std::vector<std::shared_ptr<wpst>>>>
//            wpsi(std::vector<std::shared_ptr<wpst>> &&l) { return hub<wchar_t>::psi(std::move(l)); }
//
//            inline std::shared_ptr<wpsct<std::pair<
//                    std::vector<std::shared_ptr<wpst>>,
//                    bool
//            >>> wpsd(std::vector<std::shared_ptr<wpst>> &&l, const bool &s = true) {
//                return hub<wchar_t>::psd(std::move(l), s);
//            }
//
//            inline std::shared_ptr<wplut> wplu(std::vector<typename wplt::Item> &&l) {
//                return hub<wchar_t>::plu(std::move(l));
//            }
//
//            inline std::shared_ptr<wplit> wpli(std::vector<typename wplt::Item> &&l) {
//                return hub<wchar_t>::pli(std::move(l));
//            }
//
//            inline std::shared_ptr<wpldt> wpld(std::vector<typename wplt::Item> &&l, const bool &sign = true) {
//                return hub<wchar_t>::pld(std::move(l), sign);
//            }
//
//            inline std::shared_ptr<wplct> wplc(std::vector<typename wplt::Item> &&l) {
//                return hub<wchar_t>::plc(std::move(l));
//            }
//
//            inline std::shared_ptr<wplct> wplcs(const typename Traits<wchar_t>::String &s) {
//                return hub<wchar_t>::plcs(s);
//            }
//
//            inline std::shared_ptr<wpkt> wpk(const std::shared_ptr<wpt> &p) { return hub<wchar_t>::pk(p); }
//
//            inline std::shared_ptr<wppt> wpp() { return hub<wchar_t>::pp(); }
//
//            inline std::shared_ptr<wpqt> wpq(const std::shared_ptr<wpt> &p) { return hub<wchar_t>::pq(p); }
//        }
//    }
}