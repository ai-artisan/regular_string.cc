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
        struct BinarySome : Record<Character> {
            const bool index;
            const std::shared_ptr<Record<Character>> some;

            BinarySome(const decltype(Record<Character>::end) &end, const decltype(index) &index, const decltype(some) &some) :
                    Record<Character>(end), index(index), some(some) {}
        };

        template<typename Character>
        struct BinaryEvery : Record<Character> {
            const std::array<std::shared_ptr<Record<Character>>, 2> every;

            BinaryEvery(const decltype(Record<Character>::end) &end, decltype(every) &&every) :
                    Record<Character>(end), every(std::move(every)) {}
        };

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
        struct Binary : Pattern<Character> {
            const std::array<std::shared_ptr<Pattern<Character>>, 2> binary;

            explicit Binary(decltype(binary) &&binary) : binary(std::move(binary)) {}
        };

        namespace binary {
            template<typename Character>
            struct Union : Binary<Character> {
                explicit Union(decltype(Binary<Character>::binary) &&binary) : Binary<Character>(std::move(binary)) {}

                typename Pattern<Character>::Matched match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const final;
            };

            template<typename Character>
            struct Intersection : Binary<Character> {
                explicit Intersection(decltype(Binary<Character>::binary) &&binary) : Binary<Character>(std::move(binary)) {}

                typename Pattern<Character>::Matched match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const final;
            };

            template<typename Character>
            struct Difference : Binary<Character> {
                explicit Difference(decltype(Binary<Character>::binary) &&binary) : Binary<Character>(std::move(binary)) {}

                typename Pattern<Character>::Matched match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const final;
            };

            template<typename Character>
            struct Concatenation : Binary<Character> {
                explicit Concatenation(decltype(Binary<Character>::binary) &&binary) : Binary<Character>(std::move(binary)) {}

                typename Pattern<Character>::Matched match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const final;
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
        using rbst=record::BinarySome<Character>;
        using rbet=record::BinaryEvery<Character>;
        using rlst=record::LinearSome<Character>;
        using rlet=record::LinearEvery<Character>;
        using rkt=record::KleeneClosure<Character>;

        using pt=Pattern<Character>;
        using pet=pattern::Empty<Character>;
        using pst=pattern::Singleton<Character>;
        template<typename Context>
        using psct=pattern::singleton::Closure<Character, Context>;
        using pbt=pattern::Binary<Character>;
        using pbut=pattern::binary::Union<Character>;
        using pbit=pattern::binary::Intersection<Character>;
        using pbdt=pattern::binary::Difference<Character>;
        using pbct=pattern::binary::Concatenation<Character>;
        using plt=pattern::Linear<Character>;
        using plut=pattern::linear::Union<Character>;
        using plit=pattern::linear::Intersection<Character>;
        using pldt=pattern::linear::Difference<Character>;
        using plct=pattern::linear::Concatenation<Character>;
        using pkt=pattern::KleeneClosure<Character>;
        using ppt=pattern::Placeholder<Character>;
        using pqt=pattern::Collapsed<Character>;

        static std::shared_ptr<pet> pe();

        static std::shared_ptr<pst> ps(const std::function<bool(const Character &)> &);

        template<typename Context>
        static std::shared_ptr<psct<Context>> ps(Context &&, const std::function<bool(const Context &, const Character &)> &);

        static std::shared_ptr<pst> psa();

        static std::shared_ptr<psct<Character>> psc(const Character &);

        static std::shared_ptr<psct<typename Traits<Character>::String>> pss(typename Traits<Character>::String &&);

        static std::shared_ptr<psct<std::array<Character, 2>>> psr(const Character &, const Character &);

        static std::shared_ptr<psct<std::list<std::shared_ptr<pst>>>> psu(std::list<std::shared_ptr<pst>> &&);

        static std::shared_ptr<psct<std::list<std::shared_ptr<pst>>>> psi(std::list<std::shared_ptr<pst>> &&);

        static std::shared_ptr<psct< std::list<std::shared_ptr<pst>>>> psd(std::list<std::shared_ptr<pst>> &&);

        static std::shared_ptr<pbut> pbu(const std::shared_ptr<pt> &, const std::shared_ptr<pt> &);

        static std::shared_ptr<pbit> pbi(const std::shared_ptr<pt> &, const std::shared_ptr<pt> &);

        static std::shared_ptr<pbdt> pbd(const std::shared_ptr<pt> &, const std::shared_ptr<pt> &);

        static std::shared_ptr<pbct> pbc(const std::shared_ptr<pt> &, const std::shared_ptr<pt> &);

        static std::shared_ptr<pt> pbc(const typename Traits<Character>::String &);

        static std::shared_ptr<plut> plu(std::list<typename plt::Item> &&);

        static std::shared_ptr<plit> pli(std::list<typename plt::Item> &&);

        static std::shared_ptr<pldt> pld(std::list<typename plt::Item> &&);

        static std::shared_ptr<plct> plc(std::list<typename plt::Item> &&);

        static std::shared_ptr<plct> plc(const typename Traits<Character>::String &);

        static std::shared_ptr<pkt> pk(const std::shared_ptr<pt> &);

        static std::shared_ptr<ppt> pp();

        static std::shared_ptr<pqt> pq(const std::shared_ptr<pt> &);
    };
}