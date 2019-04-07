#pragma once

#include <array>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <tuple>

#include "json.cc/json.hh"

#define CONSTRUCT_NAIVE(CLASS) explicit CLASS(decltype(model) &&model) : model(std::move(model)) {}
#define CONSTRUCT_TRANSFER(CLASS, BASE) explicit CLASS(decltype(BASE::model) &&model) : BASE(std::move(model)) {}

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
    struct Record {
        virtual typename Traits<Character>::String::const_iterator end() const = 0;
    };

    namespace record {
        template<typename Character>
        struct Naive : Record<Character> {
            const std::array<typename Traits<Character>::String::const_iterator, 2> model;

            CONSTRUCT_NAIVE(Naive)

            typename Traits<Character>::String::const_iterator end() const final;
        };

        template<typename Character>
        struct Union : Record<Character> {
            const std::pair<
                    bool,
                    std::shared_ptr<Record<Character>>
            > model;

            CONSTRUCT_NAIVE(Union)

            typename Traits<Character>::String::const_iterator end() const final;
        };

        template<typename Character>
        struct Concatenation : Record<Character> {
            const std::array<std::shared_ptr<Record<Character>>, 2> model;

            CONSTRUCT_NAIVE(Concatenation)

            typename Traits<Character>::String::const_iterator end() const final;
        };

        template<typename Character>
        struct KleeneClosure : Record<Character> {
            const std::list<std::shared_ptr<Record<Character>>> model;

            CONSTRUCT_NAIVE(KleeneClosure)

            typename Traits<Character>::String::const_iterator end() const final;
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

        template<typename Character, typename Context=nullptr_t>
        struct Singleton : Pattern<Character> {
            const std::pair<
                    std::function<bool(const Character &, const Context &)>,
                    Context
            > model;

            CONSTRUCT_NAIVE(Singleton)

            std::pair<bool, std::shared_ptr<Record<Character>>> match(
                    const typename Traits<Character>::String::const_iterator &,
                    const typename Traits<Character>::String::const_iterator &
            ) const final;
        };

        template<typename Character>
        struct Binary : Pattern<Character> {
            const std::array<std::shared_ptr<Pattern<Character>>, 2> model;

            CONSTRUCT_NAIVE(Binary)
        };

        namespace binary {
            template<typename Character>
            struct Union : Binary<Character> {
                CONSTRUCT_TRANSFER(Union, Binary<Character>)

                std::pair<bool, std::shared_ptr<Record<Character>>> match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const final;
            };

            template<typename Character>
            struct Concatenation : Binary<Character> {
                CONSTRUCT_TRANSFER(Concatenation, Binary<Character>)

                std::pair<bool, std::shared_ptr<Record<Character>>> match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const final;
            };
        }

        template<typename Character>
        struct KleeneClosure : Pattern<Character> {
            std::shared_ptr<Pattern<Character>> model;

            CONSTRUCT_NAIVE(KleeneClosure)

            std::pair<bool, std::shared_ptr<Record<Character>>> match(
                    const typename Traits<Character>::String::const_iterator &,
                    const typename Traits<Character>::String::const_iterator &
            ) const final;
        };
    }
}



//namespace regular {
//
//
//    namespace pattern {

//        template<typename CharacterType>
//        struct Linear : Pattern<CharacterType> {
//            virtual ~Linear() = 0;
//
//            const std::list<std::pair<
//                    typename Linear::String,
//                    std::shared_ptr<Pattern<CharacterType>>
//            >> sequence;
//
//            Linear(decltype(sequence) &&);
//        };
//
//        namespace linear {
//            template<typename CharacterType>
//            struct Union : Linear<CharacterType> {
//                struct Record : Pattern<CharacterType>::Record {
//                    const typename Union::String key;
//                    const std::shared_ptr<typename Pattern<CharacterType>::Record> value;
//
//                    Record(decltype(key) &&key, decltype(value) &&value);
//
//                    typename Union::Json json() final;
//                };
//
//                std::shared_ptr<Record> match(const typename Union::String::const_iterator &, const typename Union::String::const_iterator &) final;
//            };
//
//
//        }
//    }
//}


#undef CONSTRUCT_NAIVE
#undef CONSTRUCT_TRANSFER