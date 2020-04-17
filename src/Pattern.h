#pragma once

#include "regular.h"

namespace regular {
    template<typename Character>
    struct Pattern : std::enable_shared_from_this<Pattern<Character>> {
        using PtrPattern = std::shared_ptr<Pattern>;
        using StringIterator = typename CharacterTraits<Character>::String::const_iterator;
        using Matched = std::pair<bool, std::shared_ptr<Record < Character>>>;

        virtual Matched match(const StringIterator &, const StringIterator &) const = 0;

        template<typename Derived>
        std::shared_ptr<Derived> as() const {
            return std::dynamic_pointer_cast<Derived>(const_cast<Pattern<Character> *>(this)->shared_from_this());
        }
    };

    namespace pattern {
        template<typename Character>
        struct EmptyString : Pattern<Character> {
            using StringIterator = typename Pattern<Character>::StringIterator;
            using Matched = typename Pattern<Character>::Matched;

            Matched match(const StringIterator &head, const StringIterator &) const final {
                return {true, std::make_shared<Record<Character>>(head, head, head)};
            }
        };

        template<typename Character>
        struct LiteralCharacter : Pattern<Character> {
            using StringIterator = typename Pattern<Character>::StringIterator;
            using Matched = typename Pattern<Character>::Matched;
            using Describe = std::function<bool(const Character &)>;

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
                using StringIterator = typename Pattern<Character>::StringIterator;
                using Matched = typename Pattern<Character>::Matched;
                using Depict = std::function<bool(const Context &, const Character &)>;

                const Context context;
                const Depict depict;

                Closure(Context context, const Depict &depict) :
                        LiteralCharacter<Character>([&](const Character &c) { return this->depict(this->context, c); }),
                        context(std::move(context)), depict(depict) {}
            };
        }

        template<typename Character>
        struct Binary : Pattern<Character> {
            using PtrPattern = typename Pattern<Character>::PtrPattern;

            const PtrPattern first, second;

            Binary(const PtrPattern &first, const PtrPattern &second) : first(first), second(second) {}
        };

        namespace binary {
            template<typename Character>
            struct Alternation : Binary<Character> {
                using PtrPattern = typename Pattern<Character>::PtrPattern;
                using StringIterator = typename Pattern<Character>::StringIterator;
                using Matched = typename Pattern<Character>::Matched;

                Alternation(const PtrPattern &first, const PtrPattern &second) : Binary<Character>(first, second) {}

                Matched match(const StringIterator &head, const StringIterator &tail) const final {
                    bool index;
                    auto matched = this->first->match(head, tail);
                    auto greedy_end = matched.second->greedy_end;
                    if (matched.first) index = false;
                    else {
                        matched = this->second->match(head, tail);
                        index = true;
                        if (matched.second->greedy_end > greedy_end) greedy_end = matched.second->greedy_end;
                    }
                    return {
                            matched.first,
                            std::make_shared<record::Some<Character>>(
                                    head, matched.second->direct_end, greedy_end,
                                    index, matched.second
                            )
                    };
                }
            };

            template<typename Character>
            struct Concatenation : Binary<Character> {
                using PtrPattern = typename Pattern<Character>::PtrPattern;
                using StringIterator = typename Pattern<Character>::StringIterator;
                using Matched = typename Pattern<Character>::Matched;

                Concatenation(const PtrPattern &first, const PtrPattern &second) : Binary<Character>(first, second) {}

                Matched match(const StringIterator &head, const StringIterator &tail) const final {
                    auto matched = this->first->match(head, tail);
                    auto success = matched.first;
                    decltype(matched.second) first_record = matched.second, second_record;
                    auto greedy_end = first_record->greedy_end;
                    if (success) {
                        matched = this->second->match(matched.second->direct_end, tail);
                        success = success && matched.first;
                        second_record = matched.second;
                        if (second_record->greedy_end > greedy_end) greedy_end = second_record->greedy_end;
                    } else second_record = nullptr;
                    return {
                            success,
                            std::make_shared<record::Every<Character>>(
                                    head, matched.second->direct_end, greedy_end,
                                    first_record, second_record
                            )
                    };
                }
            };
        }

    }
}