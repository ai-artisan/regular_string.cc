#pragma once

#include <array>
#include <codecvt>
#include <functional>
#include <list>
#include <locale>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#define TYPE(X) typename std::remove_const<decltype(X)>::type

namespace reg {
    template<typename...>
    struct Traits {
        ~Traits() = delete;
    };

    template<>
    struct Traits<char> {
        ~Traits() = delete;

        using String=std::string;

        static inline String string(const std::string &s) {
            return s;
        }
    };

    template<>
    struct Traits<wchar_t> {
        ~Traits() = delete;

        using String=std::wstring;

        static inline String string(const std::string &s) {
            return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(s);
        }
    };

    template<typename Character>
    struct Record : std::enable_shared_from_this<Record<Character>> {
        using Cursor=typename Traits<Character>::String::const_iterator;
        const Cursor begin, direct_end, greedy_end;

        Record(Cursor, Cursor, Cursor);

        template<typename Derived>
        std::shared_ptr<Derived> as() const;
    };

    namespace record {
        template<typename Character>
        struct Some : Record<Character> {
            using Cursor=typename Record<Character>::Cursor;
            using Index=std::size_t;
            using Key=typename Traits<Character>::String;
            using Value=std::shared_ptr<Record<Character>>;

            const Index index;
            const Key key;
            const Value value;

            Some(Cursor, Cursor, Cursor, Index, Key, Value);
        };

        template<typename Character>
        struct Every : Record<Character> {
            using Cursor=typename Record<Character>::Cursor;
            using Vector=std::vector<std::shared_ptr<Record<Character>>>;
            using Map=std::unordered_map<typename Traits<Character>::String, std::shared_ptr<Record<Character>>>;

            const Vector vector;
            const Map map;

            Every(Cursor, Cursor, Cursor, Vector, Map);
        };

        template<typename Character>
        struct Greedy : Record<Character> {
            using Cursor=typename Record<Character>::Cursor;
            using List=std::list<std::shared_ptr<Record<Character>>>;

            const List list;

            Greedy(Cursor, Cursor, Cursor, List);
        };
    }

    template<typename Character>
    struct Pattern : std::enable_shared_from_this<Pattern<Character>> {
        struct Matched {
            const bool success;
            const std::shared_ptr<Record<Character>> record;
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
        struct EmptyString : Pattern<Character> {
            typename Pattern<Character>::Matched match(
                    const typename Traits<Character>::String::const_iterator &,
                    const typename Traits<Character>::String::const_iterator &
            ) const final;
        };

        template<typename Character>
        struct LiteralCharacter : Pattern<Character> {
            const std::function<bool(const Character &)> describe;

            explicit LiteralCharacter(const TYPE(describe) &describe) : describe(describe) {}

            typename Pattern<Character>::Matched match(
                    const typename Traits<Character>::String::const_iterator &,
                    const typename Traits<Character>::String::const_iterator &
            ) const final;
        };

        namespace literal_character {
            template<typename Character, typename Context>
            struct Closure : LiteralCharacter<Character> {
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
                /*explicit*/ Item(Value &&value) : key(), value(std::forward<Value>(value)) {}

                Item(TYPE(key) &&key, const TYPE(value) &value) : key(std::move(key)), value(value) {}
            };

            const std::vector<Item> linear;

            explicit Linear(TYPE(linear) &&linear) : linear(std::move(linear)) {}
        };

        namespace linear {
            template<typename Character>
            struct Alternation : Linear<Character> {
                explicit Alternation(TYPE(Linear<Character>::linear) &&linear) : Linear<Character>(std::move(linear)) {}

                typename Pattern<Character>::Matched match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const final;
            };

            template<typename Character>
            struct Concatenation : Linear<Character> {
                explicit Concatenation(TYPE(Linear<Character>::linear) &&linear) : Linear<Character>(
                        std::move(linear)) {}

                typename Pattern<Character>::Matched match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const;
            };
        }

        template<typename Character>
        struct KleeneStar : public Pattern<Character> {
            std::shared_ptr<Pattern<Character>> item;

            explicit KleeneStar(const TYPE(item) &item) : item(item) {}

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
    }
}