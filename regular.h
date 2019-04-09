#pragma once

#include <array>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <tuple>

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

        const typename Traits<Character>::String::const_iterator end;

        explicit Record(const decltype(end) &end) : end(end) {}

        template<typename Derived>
        std::shared_ptr<Derived> as() const;
    };

    namespace record {
        template<typename Character>
        struct LinearSome : Record<Character> {
            const typename Traits<Character>::String key;
            const std::shared_ptr<Record<Character>> some;

            LinearSome(const decltype(Record<Character>::end) &end, decltype(key) &&key, const decltype(some) &some) :
                    Record<Character>(end), key(std::move(key)), some(some) {}
        };

        template<typename Character>
        struct LinearEvery : Record<Character> {
            const std::unordered_map<typename Traits<Character>::String, std::shared_ptr<Record<Character>>> every;

            LinearEvery(const decltype(Record<Character>::end) &end, decltype(every) &&every) :
                    Record<Character>(end), every(std::move(every)) {}
        };

        template<typename Character>
        struct KleeneClosure : Record<Character> {
            const std::list<std::shared_ptr<Record<Character>>> list;

            KleeneClosure(const decltype(Record<Character>::end) &end, decltype(list) &&list) :
                    Record<Character>(end), list(std::move(list)) {}
        };
    }

    template<typename Character>
    struct Pattern : std::enable_shared_from_this<Pattern<Character>> {
        struct Matched {
            bool success;
            std::shared_ptr<Record<Character>> record;
        };

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

            explicit Singleton(const decltype(describe) &describe) : describe(describe) {}

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

                Closure(Context &&, const decltype(depict) &);
            };
        }

        template<typename Character>
        struct Linear : Pattern<Character> {
            struct Item {
                typename Traits<Character>::String key;
                std::shared_ptr<Pattern<Character>> value;

                template<typename Value>
                /*explicit*/ Item(Value &&value) : key(0, 0), value(std::forward<Value>(value)) {}

                Item(decltype(key) &&key, const decltype(value) &value) :
                        key(std::move(key)), value(value) {}
            };

            const std::list<Item> linear;

            explicit Linear(decltype(linear) &&linear) : linear(std::move(linear)) {}
        };

        namespace linear {
            template<typename Character>
            struct Union : Linear<Character> {
                explicit Union(decltype(Linear<Character>::linear) &&linear) : Linear<Character>(std::move(linear)) {}

                typename Pattern<Character>::Matched match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const final;
            };

            template<typename Character>
            struct Intersection : Linear<Character> {
                explicit Intersection(decltype(Linear<Character>::linear) &&linear) : Linear<Character>(std::move(linear)) {}

                typename Pattern<Character>::Matched match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const final;
            };

            template<typename Character>
            struct Difference : Linear<Character> {
                explicit Difference(decltype(Linear<Character>::linear) &&linear) : Linear<Character>(std::move(linear)) {}

                typename Pattern<Character>::Matched match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const final;
            };

            template<typename Character>
            struct Concatenation : Linear<Character> {
                explicit Concatenation(decltype(Linear<Character>::linear) &&linear) : Linear<Character>(std::move(linear)) {}

                typename Pattern<Character>::Matched match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const final;
            };
        }

        template<typename Character>
        struct KleeneClosure : Pattern<Character> {
            std::shared_ptr<Pattern<Character>> item;

            explicit KleeneClosure(const decltype(item) &item) : item(item) {}

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

            explicit Collapsed(const decltype(core) &core) : core(core) {};

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
        using rlet=record::LinearEvery<Character>;
        using rkt=record::KleeneClosure<Character>;

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

        static std::shared_ptr<pot> po();

        static std::shared_ptr<pst> ps(const std::function<bool(const Character &)> &);

        template<typename Context>
        static std::shared_ptr<psct<Context>> ps(Context &&, const std::function<bool(const Context &, const Character &)> &);

        static std::shared_ptr<pst> psa();

        static std::shared_ptr<psct<Character>> psc(const Character &);

        static std::shared_ptr<psct<typename Traits<Character>::String>> pss(typename Traits<Character>::String &&);

        static std::shared_ptr<psct<std::array<Character, 2>>> psr(const Character &, const Character &);

        static std::shared_ptr<psct<std::list<std::shared_ptr<pst>>>> psu(std::list<std::shared_ptr<pst>> &&);

        static std::shared_ptr<psct<std::list<std::shared_ptr<pst>>>> psi(std::list<std::shared_ptr<pst>> &&);

        static std::shared_ptr<psct<std::list<std::shared_ptr<pst>>>> psd(std::list<std::shared_ptr<pst>> &&);

        static std::shared_ptr<plut> plu(std::list<typename plt::Item> &&);

        static std::shared_ptr<plit> pli(std::list<typename plt::Item> &&);

        static std::shared_ptr<pldt> pld(std::list<typename plt::Item> &&);

        static std::shared_ptr<plct> plc(std::list<typename plt::Item> &&);

        static std::shared_ptr<plct> plc(const typename Traits<Character>::String &);

        static std::shared_ptr<pkt> pk(const std::shared_ptr<pt> &);

        static std::shared_ptr<ppt> pp();

        static std::shared_ptr<pqt> pq(const std::shared_ptr<pt> &);
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

            inline std::shared_ptr<pot> pe() { return hub<char>::po(); }

            inline std::shared_ptr<pst> ps(const std::function<bool(const char &)> &f) { return hub<char>::ps(f); }

            template<typename Context>
            inline std::shared_ptr<psct<Context>> ps(Context &&c, const std::function<bool(const Context &, const char &)> &f) { return hub<char>::ps(c, f); }

            inline std::shared_ptr<pst> psa() { return hub<char>::psa(); }

            inline std::shared_ptr<psct<char>> psc(const char &c) { return hub<char>::psc(c); }

            inline std::shared_ptr<psct<typename Traits<char>::String>> pss(typename Traits<char>::String &&s) { return hub<char>::pss(std::move(s)); }

            inline std::shared_ptr<psct<std::array<char, 2>>> psr(const char &c, const char &d) { return hub<char>::psr(c, d); }

            inline std::shared_ptr<psct<std::list<std::shared_ptr<pst>>>> psu(std::list<std::shared_ptr<pst>> &&l) { return hub<char>::psu(std::move(l)); }

            inline std::shared_ptr<psct<std::list<std::shared_ptr<pst>>>> psi(std::list<std::shared_ptr<pst>> &&l) { return hub<char>::psi(std::move(l)); }

            inline std::shared_ptr<psct<std::list<std::shared_ptr<pst>>>> psd(std::list<std::shared_ptr<pst>> &&l) { return hub<char>::psd(std::move(l)); }

            inline std::shared_ptr<plut> plu(std::list<typename plt::Item> &&l) { return hub<char>::plu(std::move(l)); }

            inline std::shared_ptr<plit> pli(std::list<typename plt::Item> &&l) { return hub<char>::pli(std::move(l)); }

            inline std::shared_ptr<pldt> pld(std::list<typename plt::Item> &&l) { return hub<char>::pld(std::move(l)); }

            inline std::shared_ptr<plct> plc(std::list<typename plt::Item> &&l) { return hub<char>::plc(std::move(l)); }

            inline std::shared_ptr<plct> plc(const typename Traits<char>::String &s) { return hub<char>::plc(s); }

            inline std::shared_ptr<pkt> pk(const std::shared_ptr<pt> &p) { return hub<char>::pk(p); }

            inline std::shared_ptr<ppt> pp() { return hub<char>::pp(); }

            inline std::shared_ptr<pqt> pq(const std::shared_ptr<pt> &p) { return hub<char>::pq(p); }
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

            inline std::shared_ptr<wpot> wpe() { return hub<wchar_t>::po(); }

            inline std::shared_ptr<wpst> wps(const std::function<bool(const wchar_t &)> &f) { return hub<wchar_t>::ps(f); }

            template<typename Context>
            inline std::shared_ptr<wpsct<Context>> wps(Context &&c, const std::function<bool(const Context &, const wchar_t &)> &f) { return hub<wchar_t>::ps(c, f); }

            inline std::shared_ptr<wpst> wpsa() { return hub<wchar_t>::psa(); }

            inline std::shared_ptr<wpsct<wchar_t>> wpsc(const wchar_t &c) { return hub<wchar_t>::psc(c); }

            inline std::shared_ptr<wpsct<typename Traits<wchar_t>::String>> wpss(typename Traits<wchar_t>::String &&s) { return hub<wchar_t>::pss(std::move(s)); }

            inline std::shared_ptr<wpsct<std::array<wchar_t, 2>>> wpsr(const wchar_t &c, const wchar_t &d) { return hub<wchar_t>::psr(c, d); }

            inline std::shared_ptr<wpsct<std::list<std::shared_ptr<wpst>>>> wpsu(std::list<std::shared_ptr<wpst>> &&l) { return hub<wchar_t>::psu(std::move(l)); }

            inline std::shared_ptr<wpsct<std::list<std::shared_ptr<wpst>>>> wpsi(std::list<std::shared_ptr<wpst>> &&l) { return hub<wchar_t>::psi(std::move(l)); }

            inline std::shared_ptr<wpsct<std::list<std::shared_ptr<wpst>>>> wpsd(std::list<std::shared_ptr<wpst>> &&l) { return hub<wchar_t>::psd(std::move(l)); }

            inline std::shared_ptr<wplut> wplu(std::list<typename wplt::Item> &&l) { return hub<wchar_t>::plu(std::move(l)); }

            inline std::shared_ptr<wplit> wpli(std::list<typename wplt::Item> &&l) { return hub<wchar_t>::pli(std::move(l)); }

            inline std::shared_ptr<wpldt> wpld(std::list<typename wplt::Item> &&l) { return hub<wchar_t>::pld(std::move(l)); }

            inline std::shared_ptr<wplct> wplc(std::list<typename wplt::Item> &&l) { return hub<wchar_t>::plc(std::move(l)); }

            inline std::shared_ptr<wplct> wplc(const typename Traits<wchar_t>::String &s) { return hub<wchar_t>::plc(s); }

            inline std::shared_ptr<wpkt> wpk(const std::shared_ptr<wpt> &p) { return hub<wchar_t>::pk(p); }

            inline std::shared_ptr<wppt> wpp() { return hub<wchar_t>::pp(); }

            inline std::shared_ptr<wpqt> wpq(const std::shared_ptr<wpt> &p) { return hub<wchar_t>::pq(p); }
        }
    }
}