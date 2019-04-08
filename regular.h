#pragma once

#include <array>
#include <functional>
#include <initializer_list>
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
    struct Pattern {
        struct Matched {
            bool success;
            std::shared_ptr<Record<Character>> record;
        };

        virtual typename Pattern<Character>::Matched match(
                const typename Traits<Character>::String::const_iterator &,
                const typename Traits<Character>::String::const_iterator &
        ) const = 0;
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
                /*explicit*/ Item(Value &&value) : key(""), value(std::forward<Value>(value)) {}

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
    }

    namespace shortcut {
        using r=Record<char>;
        using rbs=record::BinarySome<char>;
        using rbe=record::BinaryEvery<char>;
        using rls=record::LinearSome<char>;
        using rk=record::KleeneClosure<char>;

        std::shared_ptr<pattern::Empty<char>> pe();

        std::shared_ptr<pattern::Singleton<char>> ps(const std::function<bool(const char &)> &);

        template<typename Context>
        std::shared_ptr<pattern::singleton::Closure<char, Context>> ps(Context &&, const decltype(pattern::singleton::Closure<char, Context>::depict) &);

        std::shared_ptr<pattern::Singleton<char>> psa();

        std::shared_ptr<pattern::singleton::Closure<char, char>> psc(const char &);

        std::shared_ptr<pattern::singleton::Closure<char, Traits<char>::String>> pss(Traits<char>::String &&);

        std::shared_ptr<pattern::singleton::Closure<char, std::array<char, 2>>> psr(const char &, const char &);

        std::shared_ptr<pattern::singleton::Closure<
                char,
                std::list<std::shared_ptr<pattern::Singleton<char>>>
        >> psu(std::initializer_list<std::shared_ptr<pattern::Singleton<char>>>);

        std::shared_ptr<pattern::singleton::Closure<
                char,
                std::list<std::shared_ptr<pattern::Singleton<char>>>
        >> psi(std::initializer_list<std::shared_ptr<pattern::Singleton<char>>>);

        std::shared_ptr<pattern::singleton::Closure<
                char,
                std::list<std::shared_ptr<pattern::Singleton<char>>>
        >> psd(std::initializer_list<std::shared_ptr<pattern::Singleton<char>>>);

        std::shared_ptr<pattern::binary::Union<char>> pu(const std::shared_ptr<Pattern<char>> &, const std::shared_ptr<Pattern<char>> &);

        std::shared_ptr<pattern::binary::Intersection<char>> pi(const std::shared_ptr<Pattern<char>> &, const std::shared_ptr<Pattern<char>> &);

        std::shared_ptr<pattern::binary::Difference<char>> pd(const std::shared_ptr<Pattern<char>> &, const std::shared_ptr<Pattern<char>> &);

        std::shared_ptr<pattern::binary::Concatenation<char>> pc(const std::shared_ptr<Pattern<char>> &, const std::shared_ptr<Pattern<char>> &);

        std::shared_ptr<pattern::linear::Union<char>> pu(std::initializer_list<pattern::Linear<char>::Item>);

        std::shared_ptr<pattern::KleeneClosure<char>> pk(const std::shared_ptr<Pattern<char>> &);
    }
}