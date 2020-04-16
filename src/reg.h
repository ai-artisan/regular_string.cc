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

namespace reg {

    template<typename Character>
    struct Record : std::enable_shared_from_this<Record<Character>> {
        using Cursor=typename Traits<Character>::String::const_iterator;
        const Cursor begin, direct_end, greedy_end;

        Record(Cursor begin, Cursor direct_end, Cursor greedy_end) :
                begin(std::move(begin)), direct_end(std::move(direct_end)), greedy_end(std::move(greedy_end)) {}

        template<typename Derived>
        std::shared_ptr<Derived> as() const;
    };

    namespace record {
        template<typename Character>
        struct Binary : Record<Character> {
            using Cursor=typename Record<Character>::Cursor;
            using Array=std::array<std::shared_ptr<Record<Character>>, 2>;
            const Array array;

            Binary(Cursor begin, Cursor direct_end, Cursor greedy_end, Array array) :
                    Record<Character>(std::move(begin), std::move(direct_end), std::move(greedy_end)),
                    array(std::move(array)) {}
        };

        template<typename Character>
        struct LinearSome : Record<Character> {
            using Cursor=typename Record<Character>::Cursor;
            using Index=std::size_t;
            using Key=typename Traits<Character>::String;
            using Value=std::shared_ptr<Record<Character>>;

            const Index index;
            const Key key;
            const Value value;

            LinearSome(Cursor begin, Cursor direct_end, Cursor greedy_end, const Index &index, Key key, Value value) :
                    Record<Character>(std::move(begin), std::move(direct_end), std::move(greedy_end)),
                    index(index), key(std::move(key)), value(std::move(value)) {}
        };

        template<typename Character>
        struct LinearEvery : Record<Character> {
            using Cursor=typename Record<Character>::Cursor;
            using Vector=std::vector<std::shared_ptr<Record<Character>>>;
            using Map=std::unordered_map<typename Traits<Character>::String, std::shared_ptr<Record<Character>>>;

            const Vector vector;
            const Map map;

            LinearEvery(Cursor begin, Cursor direct_end, Cursor greedy_end, Vector vector, Map map) :
                    Record<Character>(std::move(begin), std::move(direct_end), std::move(greedy_end)),
                    vector(std::move(vector)), map(std::move(map)) {}
        };

        template<typename Character>
        struct Greedy : Record<Character> {
            using Cursor=typename Record<Character>::Cursor;
            using List=std::list<std::shared_ptr<Record<Character>>>;

            const List list;

            Greedy(Cursor begin, Cursor direct_end, Cursor greedy_end, List list) :
                    Record<Character>(std::move(begin), std::move(direct_end), std::move(greedy_end)),
                    list(std::move(list)) {}
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
            using Describe=std::function<bool(const Character &)>;

            const Describe describe;

            explicit LiteralCharacter(Describe describe) : describe(std::move(describe)) {}

            typename Pattern<Character>::Matched match(
                    const typename Traits<Character>::String::const_iterator &,
                    const typename Traits<Character>::String::const_iterator &
            ) const final;
        };

        namespace literal_character {
            template<typename Character, typename Context>
            struct Closure : LiteralCharacter<Character> {
                using Depict=std::function<bool(const Context &, const Character &)>;

                const Context context;
                const Depict depict;

                Closure(Context, Depict);
            };
        }

        template<typename Character>
        struct Linear : Pattern<Character> {
            struct Item {
                using Key=typename Traits<Character>::String;
                using Value=std::shared_ptr<Pattern<Character>>;

                Key key;
                Value value;

                template<typename Value>
                /*explicit*/ Item(Value value) : key(), value(std::move(value)) {}

                Item(Key key, Value value) : key(std::move(key)), value(std::move(value)) {}
            };

            using Value=std::vector<Item>;

            const Value value;

            explicit Linear(Value value) : value(std::move(value)) {}
        };

        namespace linear {
            template<typename Character>
            struct Alternation : Linear<Character> {
                using Value=typename Linear<Character>::Value;

                explicit Alternation(Value value) : Linear<Character>(std::move(value)) {}

                typename Pattern<Character>::Matched match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const final;
            };

            template<typename Character>
            struct Concatenation : Linear<Character> {
                using Value=typename Linear<Character>::Value;

                explicit Concatenation(Value value) : Linear<Character>(std::move(value)) {}

                typename Pattern<Character>::Matched match(
                        const typename Traits<Character>::String::const_iterator &,
                        const typename Traits<Character>::String::const_iterator &
                ) const;
            };
        }

        template<typename Character>
        struct KleeneStar : public Pattern<Character> {
            using Item=std::shared_ptr<Pattern<Character>>;
            const Item item;

            explicit KleeneStar(Item item) : item(std::move(item)) {}

            typename Pattern<Character>::Matched match(
                    const typename Traits<Character>::String::const_iterator &,
                    const typename Traits<Character>::String::const_iterator &
            ) const final;
        };

        template<typename Character>
        struct Operation : Pattern<Character> {
            using Array=std::array<std::shared_ptr<Pattern<Character>>, 2>;
            const bool sign;
            const Array array;

            Operation(const bool &sign, Array array) : sign(sign), array(std::move(array)) {}

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
            using Core=std::shared_ptr<Pattern<Character>>;
            const Core core;

            explicit Collapsed(Core core) : core(std::move(core)) {};

            typename Pattern<Character>::Matched match(
                    const typename Traits<Character>::String::const_iterator &,
                    const typename Traits<Character>::String::const_iterator &
            ) const final;
        };
    }
}