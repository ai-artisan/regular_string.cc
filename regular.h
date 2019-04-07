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
        const typename Traits<Character>::String::const_iterator end;

        explicit Record(const decltype(end) &end) : end(end) {}

        template<typename Derived>
        std::shared_ptr<Derived> as() const;
    };

    namespace record {
        template<typename Character>
        struct SetBinary : Record<Character> {
            const bool index;
            const std::shared_ptr<Record<Character>> value;

            SetBinary(const decltype(Record<Character>::end) &end, const decltype(index) &index, const decltype(value) &value) :
                    Record<Character>(end), index(index), value(value) {}
        };

        template<typename Character>
        struct ConcatenationBinary : Record<Character> {
            const std::array<std::shared_ptr<Record<Character>>, 2> binary;

            ConcatenationBinary(const decltype(Record<Character>::end) &end, decltype(binary) &&binary) :
                    Record<Character>(end), binary(std::move(binary)) {}
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
        virtual std::pair<bool, std::shared_ptr<Record<Character>>> match(
                const typename Traits<Character>::String::const_iterator &,
                const typename Traits<Character>::String::const_iterator &
        ) const = 0;
    };

    namespace pattern {
        template<typename Character>
        struct Empty : Pattern<Character> {
            std::pair<bool, std::shared_ptr<Record<Character>>> match(
                    const typename Traits<Character>::String::const_iterator &,
                    const typename Traits<Character>::String::const_iterator &
            ) const final;
        };

        template<typename Character, typename Context>
        struct Singleton : Pattern<Character> {

            std::function<bool(const Character &, const Context &)> describe;
            Context context;

            Singleton(const decltype(describe) &describe, Context &&context) :
                    describe(describe), context(std::move(context)) {}

            std::pair<bool, std::shared_ptr<Record<Character>>> match(
                    const typename Traits<Character>::String::const_iterator &,
                    const typename Traits<Character>::String::const_iterator &
            ) const final;
        };

        namespace singleton {

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

                std::pair<bool, std::shared_ptr<Record<Character>>> match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const final;
            };

            template<typename Character>
            struct Concatenation : Binary<Character> {
                explicit Concatenation(decltype(Binary<Character>::binary) &&binary) : Binary<Character>(std::move(binary)) {}

                std::pair<bool, std::shared_ptr<Record<Character>>> match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const final;
            };

            template<typename Character>
            struct Intersection : Binary<Character> {
                explicit Intersection(decltype(Binary<Character>::binary) &&binary) : Binary<Character>(std::move(binary)) {}

                std::pair<bool, std::shared_ptr<Record<Character>>> match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const final;
            };

            template<typename Character>
            struct Complement : Binary<Character> {
                explicit Complement(decltype(Binary<Character>::binary) &&binary) : Binary<Character>(std::move(binary)) {}

                std::pair<bool, std::shared_ptr<Record<Character>>> match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const final;
            };
        }

        template<typename Character>
        struct Linear : Pattern<Character> {
            std::list<std::pair<
                    typename Traits<Character>::String,
                    std::shared_ptr<Pattern<Character>>
            >> linear;

            explicit Linear(decltype(linear) &&linear) : linear(std::move(linear)) {}
        };

//        namespace

        template<typename Character>
        struct KleeneClosure : Pattern<Character> {
            std::shared_ptr<Pattern<Character>> item;

            explicit KleeneClosure(const decltype(item) &item) : item(item) {}

            std::pair<bool, std::shared_ptr<Record<Character>>> match(
                    const typename Traits<Character>::String::const_iterator &,
                    const typename Traits<Character>::String::const_iterator &
            ) const final;
        };
    }

    namespace shortcut {
        using r=Record<char>;
        using rsb=record::SetBinary<char>;
        using rcb=record::ConcatenationBinary<char>;
        using rkc=record::KleeneClosure<char>;

        std::shared_ptr<pattern::Empty<char>> pe();

//        template<typename Context=nullptr_t>
//        std::shared_ptr<pattern::Singleton<char, Context>> ps(const std::function<bool(const char &, const Context &)> &, Context && = nullptr);
//
//        std::shared_ptr<pattern::Singleton<char, nullptr_t>> psa();
//
//        std::shared_ptr<pattern::Singleton<char, std::pair<char, bool>>> psi(const char &, const bool & = true);
//
//        std::shared_ptr<pattern::Singleton<char, std::pair<Traits<char>::String, bool>>> psi(const Traits<char>::String &, const bool & = true);
//
//        std::shared_ptr<pattern::Singleton<char, std::pair<char, char>>> psr();
    }
}