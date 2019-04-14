#pragma once

#include <array>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <tuple>

#define TYPE(X) typename std::remove_const<decltype(X)>::type

namespace regular {
    template<typename...>
    struct Traits {
        ~Traits() = delete;
    };

    template<>
    struct Traits<char> {
        ~Traits() = delete;

        using String=std::string;
    };

    template<>
    struct Traits<wchar_t> {
        ~Traits() = delete;

        using String=std::wstring;
    };

    template<typename Character>
    struct Record : std::enable_shared_from_this<Record<Character>> {
        virtual ~Record() = default;

        typename Traits<Character>::String::const_iterator begin, end;

        inline typename Traits<Character>::String string() const {
            return typename Traits<Character>::String(begin, end);
        }

        template<typename Derived>
        std::shared_ptr<Derived> as() const;
    };

    namespace record {
        template<typename Character>
        struct LinearSome : Record<Character> {
            typename Traits<Character>::String key;
            std::shared_ptr<Record<Character>> value;
        };

        template<typename Character>
        struct Kleene : Record<Character> {
            std::list<std::shared_ptr<Record<Character>>> list;
        };

        namespace list {
            template<typename Character>
            struct LinearEvery : Kleene<Character> {
                std::unordered_map<typename Traits<Character>::String, std::shared_ptr<Record<Character>>> map;
            };
        }
    }

    template<typename Character>
    struct Pattern : std::enable_shared_from_this<Pattern<Character>> {
        struct Matched {
            bool success;
            std::shared_ptr<Record<Character>> record;
        };

        inline typename Pattern<Character>::Matched adapt(const typename Traits<Character>::String &s) const {
            return match(s.cbegin(), s.cend());
        }

        virtual typename Pattern<Character>::Matched match(
                const typename Traits<Character>::String::const_iterator &,
                const typename Traits<Character>::String::const_iterator &
        ) const = 0;

        template<typename Derived>
        std::shared_ptr<Derived> as() const;
    };

    namespace pattern {
        template<typename Character>
        struct Empty : Pattern<Character> {
            typename Pattern<Character>::Matched match(
                    const typename Traits<Character>::String::const_iterator &,
                    const typename Traits<Character>::String::const_iterator &
            ) const final;
        };

        template<typename Character>
        struct Singleton : Pattern<Character> {
            const std::function<bool(const Character &)> describe;

            explicit Singleton(const TYPE(describe) &describe) : describe(describe) {}

            typename Pattern<Character>::Matched match(
                    const typename Traits<Character>::String::const_iterator &,
                    const typename Traits<Character>::String::const_iterator &
            ) const final;
        };

        namespace singleton {
            template<typename Character, typename Context>
            struct Closure : Singleton<Character> {
                const Context context;
                const std::function<bool(const Context &, const Character &)> depict;

                Closure(Context &&, const TYPE(depict) &);
            };
        }

        template<typename Character>
        struct Linear : Pattern<Character> {
            struct Item {
                typename Traits<Character>::String key;
                std::shared_ptr<Pattern<Character>> value;

                template<typename Value>
                /*explicit*/ Item(Value &&value) : key(0, 0), value(std::forward<Value>(value)) {}

                Item(TYPE(key) &&key, const TYPE(value) &value) :
                        key(std::move(key)), value(value) {}
            };

            const std::list<Item> linear;

            explicit Linear(TYPE(linear) &&linear) : linear(std::move(linear)) {}
        };

        namespace linear {
            template<typename Character>
            struct Union : Linear<Character> {
                explicit Union(TYPE(Linear<Character>::linear) &&linear) : Linear<Character>(std::move(linear)) {}

                typename Pattern<Character>::Matched match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const final;
            };

            template<typename Character>
            struct Intersection : Linear<Character> {
                explicit Intersection(TYPE(Linear<Character>::linear) &&linear) : Linear<Character>(std::move(linear)) {}

                typename Pattern<Character>::Matched match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const;
            };

            template<typename Character>
            struct Difference : Linear<Character> {
                const bool sign;

                explicit Difference(TYPE(Linear<Character>::linear) &&linear, const TYPE(sign) &sign = true) :
                        Linear<Character>(std::move(linear)), sign(sign) {}

                typename Pattern<Character>::Matched match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const final;
            };

            template<typename Character>
            struct Concatenation : Linear<Character> {
                explicit Concatenation(TYPE(Linear<Character>::linear) &&linear) : Linear<Character>(std::move(linear)) {}

                typename Pattern<Character>::Matched match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const;
            };
        }

        template<typename Character>
        struct KleeneClosure : Pattern<Character> {
            std::shared_ptr<Pattern<Character>> item;

            explicit KleeneClosure(const TYPE(item) &item) : item(item) {}

            typename Pattern<Character>::Matched match(
                    const typename Traits<Character>::String::const_iterator &,
                    const typename Traits<Character>::String::const_iterator &
            ) const final;
        };

        template<typename Character>
        struct Placeholder : Pattern<Character> {
            std::shared_ptr<Pattern<Character>> place;

            typename Pattern<Character>::Matched match(
                    const typename Traits<Character>::String::const_iterator &,
                    const typename Traits<Character>::String::const_iterator &
            ) const final;
        };

        template<typename Character>
        struct Collapsed : Pattern<Character> {
            const std::shared_ptr<Pattern<Character>> core;

            explicit Collapsed(const TYPE(core) &core) : core(core) {};

            typename Pattern<Character>::Matched match(
                    const typename Traits<Character>::String::const_iterator &,
                    const typename Traits<Character>::String::const_iterator &
            ) const final;
        };

        template<typename Character>
        struct Custom : Pattern<Character> {
            const std::shared_ptr<Pattern<Character>> base;
            const std::function<std::shared_ptr<Record<Character>>(const std::shared_ptr<Record<Character>> &)> process;

            Custom(const TYPE(base) &base, const TYPE(process) &process) :
                    base(base), process(process) {}

            typename Pattern<Character>::Matched match(
                    const typename Traits<Character>::String::const_iterator &,
                    const typename Traits<Character>::String::const_iterator &
            ) const final;
        };
    }

    template<typename Character>
    struct hub {
        ~hub() = delete;

        using rt=Record<Character>;
        using rlst=record::LinearSome<Character>;
        using rlet=record::list::LinearEvery<Character>;
        using rkt=record::Kleene<Character>;

        using pt=Pattern<Character>;
        using pot=pattern::Empty<Character>;
        using pst=pattern::Singleton<Character>;
        template<typename Context>
        using psct=pattern::singleton::Closure<Character, Context>;
        using plt=pattern::Linear<Character>;
        using plut=pattern::linear::Union<Character>;
        using plit=pattern::linear::Intersection<Character>;
        using pldt=pattern::linear::Difference<Character>;
        using plct=pattern::linear::Concatenation<Character>;
        using pkt=pattern::KleeneClosure<Character>;
        using ppt=pattern::Placeholder<Character>;
        using pqt=pattern::Collapsed<Character>;
        using pct=pattern::Custom<Character>;

        static inline std::shared_ptr<pot> po() {
            return std::make_shared<pot>();
        }

        static inline std::shared_ptr<pst> ps(const std::function<bool(const Character &)> &describe) {
            return std::make_shared<pst>(describe);
        }

        template<typename Context>
        static inline std::shared_ptr<psct<Context>> ps(Context &&context, const std::function<bool(const Context &, const Character &)> &depict) {
            return std::make_shared<hub<Character>::psct<Context>>(std::forward<Context>(context), depict);
        }

        static inline std::shared_ptr<pst> psa() {
            return std::make_shared<pst>([](const Character &) {
                return true;
            });
        }

        static inline std::shared_ptr<psct<Character>> psc(const Character &c0) {
            return std::make_shared<hub<Character>::psct<Character>>(Character(c0), [](const Character &c0, const Character &c) -> bool {
                return c == c0;
            });
        }

        static std::shared_ptr<psct<typename Traits<Character>::String>> pss(typename Traits<Character>::String &&s) {
            return std::make_shared<hub<Character>::psct<typename Traits<Character>::String>>(std::move(s), [&](const typename Traits<Character>::String &s, const Character &c) -> bool {
                for (auto i = s.cbegin(); i != s.cend(); ({
                    if (c == *i) return true;
                    i++;
                }));
                return false;
            });
        }

        static inline std::shared_ptr<psct<std::array<Character, 2>>> psr(const Character &inf, const Character &sup) {
            return std::make_shared<hub<Character>::psct<std::array<Character, 2>>>(std::array{inf, sup}, [&](const std::array<Character, 2> &interval, const Character &c) -> bool {
                return interval[0] <= c && c <= interval[1];
            });
        }

        static std::shared_ptr<psct<std::list<std::shared_ptr<pst>>>> psu(std::list<std::shared_ptr<pst>> &&list) {
            return std::make_shared<pattern::singleton::Closure<
                    Character,
                    std::list<std::shared_ptr<pst>>
            >>(std::move(list), [&](const std::list<std::shared_ptr<pst>> &list, const Character &c) -> bool {
                for (auto i = list.cbegin(); i != list.cend(); ({
                    if ((*i)->describe(c)) return true;
                    i++;
                }));
                return false;
            });
        }

        static std::shared_ptr<psct<std::list<std::shared_ptr<pst>>>> psi(std::list<std::shared_ptr<typename hub<Character>::pst>> &&list) {
            return std::make_shared<pattern::singleton::Closure<
                    Character,
                    std::list<std::shared_ptr<pst>>
            >>(std::move(list), [&](const std::list<std::shared_ptr<pst>> &list, const Character &c) -> bool {
                for (auto i = list.cbegin(); i != list.cend(); ({
                    if (!(*i)->describe(c)) return false;
                    i++;
                }));
                return true;
            });
        }

        static std::shared_ptr<psct<std::pair<
                std::list<std::shared_ptr<pst>>,
                bool
        >>> psd(std::list<std::shared_ptr<typename hub<Character>::pst>> &&list, const bool &sign) {
            return std::make_shared<hub::psct<std::pair<
                    std::list<std::shared_ptr<hub::pst>>,
                    bool
            >>>(std::pair{std::move(list), sign}, [&](const std::pair<
                    std::list<std::shared_ptr<hub::pst>>,
                    bool
            > &pair, const Character &c) -> bool {
                auto&[l, s]=pair;
                bool b = false;
                for (auto i = l.crbegin(); i != l.crend(); ({
                    b = (*i)->describe(c) && !b;
                    i++;
                }));
                return !(b xor s);
            });
        }

        static inline std::shared_ptr<plut> plu(std::list<typename plt::Item> &&list) {
            return std::make_shared<plut>(std::move(list));
        }

        static inline std::shared_ptr<plit> pli(std::list<typename plt::Item> &&list) {
            return std::make_shared<plit>(std::move(list));
        }

        static inline std::shared_ptr<pldt> pld(std::list<typename plt::Item> &&list, const bool &sign) {
            return std::make_shared<pldt>(std::move(list), sign);
        }

        static inline std::shared_ptr<plct> plc(std::list<typename plt::Item> &&list) {
            return std::make_shared<plct>(std::move(list));
        }

        static std::shared_ptr<plct> plc(const typename Traits<Character>::String &s) {
            std::list<typename plt::Item> linear;
            for (auto i = s.cbegin(); i != s.cend(); ({
                linear.emplace_back(psc(*i));
                i++;
            }));
            return std::make_shared<plct>(std::move(linear));
        }

        static inline std::shared_ptr<pkt> pk(const std::shared_ptr<pt> &item) {
            return std::make_shared<pkt>(item);
        }

        static inline std::shared_ptr<ppt> pp() {
            return std::make_shared<ppt>();
        }

        static inline std::shared_ptr<pqt> pq(const std::shared_ptr<pt> &p) {
            return std::make_shared<pqt>(p);
        }

        static inline std::shared_ptr<pct> pc(const std::shared_ptr<pt> &base, const TYPE(pct::process) &process) {
            return std::make_shared<pct>(base, process);
        }
    };

    namespace shortcut {
        namespace narrow {
            using rt=hub<char>::rt;
            using rlst=hub<char>::rlst;
            using rlet=hub<char>::rlet;
            using rkt=hub<char>::rkt;

            using pt=hub<char>::pt;
            using pot=hub<char>::pot;
            using pst=hub<char>::pst;
            template<typename Context>
            using psct=hub<char>::psct<Context>;
            using plt=hub<char>::plt;
            using plut=hub<char>::plut;
            using plit=hub<char>::plit;
            using pldt=hub<char>::pldt;
            using plct=hub<char>::plct;
            using pkt=hub<char>::pkt;
            using ppt=hub<char>::ppt;
            using pqt=hub<char>::pqt;
            using pct=hub<char>::pct;

            inline std::shared_ptr<pot> po() { return hub<char>::po(); }

            inline std::shared_ptr<pst> ps(const std::function<bool(const char &)> &f) { return hub<char>::ps(f); }

            template<typename Context>
            inline std::shared_ptr<psct<Context>> ps(Context &&c, const std::function<bool(const Context &, const char &)> &f) { return hub<char>::ps(c, f); }

            inline std::shared_ptr<pst> psa() { return hub<char>::psa(); }

            inline std::shared_ptr<psct<char>> psc(const char &c) { return hub<char>::psc(c); }

            inline std::shared_ptr<psct<typename Traits<char>::String>> pss(typename Traits<char>::String &&s) { return hub<char>::pss(std::move(s)); }

            inline std::shared_ptr<psct<std::array<char, 2>>> psr(const char &c, const char &d) { return hub<char>::psr(c, d); }

            inline std::shared_ptr<psct<std::list<std::shared_ptr<pst>>>> psu(std::list<std::shared_ptr<pst>> &&l) { return hub<char>::psu(std::move(l)); }

            inline std::shared_ptr<psct<std::list<std::shared_ptr<pst>>>> psi(std::list<std::shared_ptr<pst>> &&l) { return hub<char>::psi(std::move(l)); }

            inline std::shared_ptr<psct<std::pair<
                    std::list<std::shared_ptr<pst>>,
                    bool
            >>> psd(std::list<std::shared_ptr<pst>> &&l, const bool &s = true) { return hub<char>::psd(std::move(l), s); }

            inline std::shared_ptr<plut> plu(std::list<typename plt::Item> &&l) { return hub<char>::plu(std::move(l)); }

            inline std::shared_ptr<plit> pli(std::list<typename plt::Item> &&l) { return hub<char>::pli(std::move(l)); }

            inline std::shared_ptr<pldt> pld(std::list<typename plt::Item> &&l, const bool &sign) { return hub<char>::pld(std::move(l), sign); }

            inline std::shared_ptr<plct> plc(std::list<typename plt::Item> &&l) { return hub<char>::plc(std::move(l)); }

            inline std::shared_ptr<plct> plc(const typename Traits<char>::String &s) { return hub<char>::plc(s); }

            inline std::shared_ptr<pkt> pk(const std::shared_ptr<pt> &p) { return hub<char>::pk(p); }

            inline std::shared_ptr<ppt> pp() { return hub<char>::pp(); }

            inline std::shared_ptr<pqt> pq(const std::shared_ptr<pt> &p) { return hub<char>::pq(p); }

            inline std::shared_ptr<pct> pc(const std::shared_ptr<pt> &base, const TYPE(pct::process) &process) { return hub<char>::pc(base, process); }
        }
        namespace wide {
            using wrt=hub<wchar_t>::rt;
            using wrlst=hub<wchar_t>::rlst;
            using wrlet=hub<wchar_t>::rlet;
            using wrkt=hub<wchar_t>::rkt;

            using wpt=hub<wchar_t>::pt;
            using wpot=hub<wchar_t>::pot;
            using wpst=hub<wchar_t>::pst;
            template<typename Context>
            using wpsct=hub<wchar_t>::psct<Context>;
            using wplt=hub<wchar_t>::plt;
            using wplut=hub<wchar_t>::plut;
            using wplit=hub<wchar_t>::plit;
            using wpldt=hub<wchar_t>::pldt;
            using wplct=hub<wchar_t>::plct;
            using wpkt=hub<wchar_t>::pkt;
            using wppt=hub<wchar_t>::ppt;
            using wpqt=hub<wchar_t>::pqt;
            using wpct=hub<wchar_t>::pct;

            inline std::shared_ptr<wpot> wpo() { return hub<wchar_t>::po(); }

            inline std::shared_ptr<wpst> wps(const std::function<bool(const wchar_t &)> &f) { return hub<wchar_t>::ps(f); }

            template<typename Context>
            inline std::shared_ptr<wpsct<Context>> wps(Context &&c, const std::function<bool(const Context &, const wchar_t &)> &f) { return hub<wchar_t>::ps(c, f); }

            inline std::shared_ptr<wpst> wpsa() { return hub<wchar_t>::psa(); }

            inline std::shared_ptr<wpsct<wchar_t>> wpsc(const wchar_t &c) { return hub<wchar_t>::psc(c); }

            inline std::shared_ptr<wpsct<typename Traits<wchar_t>::String>> wpss(typename Traits<wchar_t>::String &&s) { return hub<wchar_t>::pss(std::move(s)); }

            inline std::shared_ptr<wpsct<std::array<wchar_t, 2>>> wpsr(const wchar_t &c, const wchar_t &d) { return hub<wchar_t>::psr(c, d); }

            inline std::shared_ptr<wpsct<std::list<std::shared_ptr<wpst>>>> wpsu(std::list<std::shared_ptr<wpst>> &&l) { return hub<wchar_t>::psu(std::move(l)); }

            inline std::shared_ptr<wpsct<std::list<std::shared_ptr<wpst>>>> wpsi(std::list<std::shared_ptr<wpst>> &&l) { return hub<wchar_t>::psi(std::move(l)); }

            inline std::shared_ptr<wpsct<std::pair<
                    std::list<std::shared_ptr<wpst>>,
                    bool
            >>> wpsd(std::list<std::shared_ptr<wpst>> &&l, const bool &s = true) { return hub<wchar_t>::psd(std::move(l), s); }

            inline std::shared_ptr<wplut> wplu(std::list<typename wplt::Item> &&l) { return hub<wchar_t>::plu(std::move(l)); }

            inline std::shared_ptr<wplit> wpli(std::list<typename wplt::Item> &&l) { return hub<wchar_t>::pli(std::move(l)); }

            inline std::shared_ptr<wpldt> wpld(std::list<typename wplt::Item> &&l, const bool &sign) { return hub<wchar_t>::pld(std::move(l), sign); }

            inline std::shared_ptr<wplct> wplc(std::list<typename wplt::Item> &&l) { return hub<wchar_t>::plc(std::move(l)); }

            inline std::shared_ptr<wplct> wplc(const typename Traits<wchar_t>::String &s) { return hub<wchar_t>::plc(s); }

            inline std::shared_ptr<wpkt> wpk(const std::shared_ptr<wpt> &p) { return hub<wchar_t>::pk(p); }

            inline std::shared_ptr<wppt> wpp() { return hub<wchar_t>::pp(); }

            inline std::shared_ptr<wpqt> wpq(const std::shared_ptr<wpt> &p) { return hub<wchar_t>::pq(p); }

            inline std::shared_ptr<wpct> pc(const std::shared_ptr<wpt> &base, const TYPE(wpct::process) &process) { return hub<wchar_t>::pc(base, process); }
        }
    }
}