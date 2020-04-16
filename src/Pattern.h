#pragma once

#include "regular.h"

namespace regular {
    template<typename Character>
    struct Pattern : std::enable_shared_from_this<Pattern<Character>> {
        using PtrPattern=std::shared_ptr<Pattern>;
        using StringIterator=typename CharacterTraits<Character>::String::const_iterator;
        using Matched=std::pair<bool, std::shared_ptr<Record < Character>>>;

        virtual Matched match(const StringIterator &, const StringIterator &) const = 0;

        template<typename Derived>
        std::shared_ptr<Derived> as() const {
            return std::dynamic_pointer_cast<Derived>(const_cast<Pattern<Character> *>(this)->shared_from_this());
        }
    };

    namespace pattern {
        template<typename Character>
        struct EmptyString : Pattern<Character> {
            using StringIterator=typename Pattern<Character>::StringIterator;
            using Matched=typename Pattern<Character>::Matched;

            Matched match(const StringIterator &head, const StringIterator &) const final {
                return {true, std::make_shared<Record<Character>>(head, head, head)};
            }
        };

        template<typename Character>
        struct LiteralCharacter : Pattern<Character> {
            using StringIterator=typename Pattern<Character>::StringIterator;
            using Matched=typename Pattern<Character>::Matched;
            using Describe=std::function<bool(const Character &)>;

            const Describe describe;

            explicit LiteralCharacter(const Describe &describe) : describe(describe) {}

            Matched match(const StringIterator &head, const StringIterator &tail) const final {
                bool success;
                auto end = head;
                if (head == tail) success = false;
                else {
                    success = describe(*head);
                    end = std::next(head);
                }
                return {success, std::make_shared<Record<Character>>(head, end, end)};
            }
        };

        namespace literal_character {
            template<typename Character, typename Context>
            struct Closure : LiteralCharacter<Character> {
                using StringIterator=typename Pattern<Character>::StringIterator;
                using Matched=typename Pattern<Character>::Matched;
                using Depict=std::function<bool(const Context &, const Character &)>;

                const Context context;
                const Depict depict;

                Closure(Context context, const Depict &depict) :
                        LiteralCharacter<Character>([&](const Character &c) { return this->depict(this->context, c); }),
                        context(std::move(context)), depict(depict) {}
            };
        }

        template<typename Character>
        struct Binary : Pattern<Character> {
            using PtrPattern=typename Pattern<Character>::PtrPattern;
            using Array=std::array<PtrPattern, 2>;

            const Array array;

            explicit Binary(Array array) : array(std::move(array)) {}
        };

        namespace binary {
            template<typename Character>
            struct Alternation : Binary<Character> {
                using PtrPattern=typename Pattern<Character>::PtrPattern;
                using StringIterator=typename Pattern<Character>::StringIterator;
                using Matched=typename Pattern<Character>::Matched;
                using Array=typename Binary<Character>::Array;

                explicit Alternation(Array array) : Binary<Character>(std::move(array)) {}

                Matched match(const StringIterator &head, const StringIterator &tail) const final {
                    bool index;
                    auto greedy_end = head;
                    auto matched = this->array[0]->match(head, tail);
                    if (matched.first) {
                        index = false;
                        greedy_end = matched.second->greedy_end;
                    } else {
                        matched = this->array[1]->match(head, tail);
                        index = true;
                        if (matched.second->greedy_end > greedy_end) greedy_end = matched.second->greedy_end;
                    }
                    return {
                            matched.first,
                            std::make_shared<record::Unary<Character>>(head, matched.second->direct_end, greedy_end, index, matched.second)
                    };
                }
            };
        }

    }
}