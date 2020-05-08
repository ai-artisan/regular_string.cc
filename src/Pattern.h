#pragma once

#include "regular.h"

namespace regular {
    template<typename Character>
    struct Pattern {
        using String = typename CharacterTraits<Character>::String;
        using StringIterator = typename String::const_iterator;
        using PtrRecord = typename Record<Character>::PtrRecord;
        using Children = typename Record<Character>::Children;

        virtual PtrRecord match(const StringIterator &, const StringIterator &) const = 0;

        PtrRecord match(const String &s) const {
            return match(s.cbegin(), s.cend());
        }
    };

    namespace pattern {
        template<typename Character>
        struct EmptyString : Pattern<Character> {
            using StringIterator = typename Pattern<Character>::StringIterator;
            using PtrRecord = typename Pattern<Character>::PtrRecord;
            using Children = typename Pattern<Character>::Children;

            using Pattern<Character>::match;

            PtrRecord match(const StringIterator &head, const StringIterator &) const final {
                return std::make_shared<Record<Character>>(true, head, head);
            };
        };

        template<typename Character>
        struct LiteralCharacter : Pattern<Character> {
            using StringIterator = typename Pattern<Character>::StringIterator;
            using PtrRecord = typename Pattern<Character>::PtrRecord;
            using Children = typename Pattern<Character>::Children;
            using Describe = std::function<bool(const Character &)>;

            const Describe describe;

            explicit LiteralCharacter(const Describe &describe) : describe(describe) {}

            using Pattern<Character>::match;

            PtrRecord match(const StringIterator &head, const StringIterator &tail) const final {
                if (head == tail) return std::make_shared<Record<Character>>(false, head, head);
                else return std::make_shared<Record<Character>>(describe(*head), head, std::next(head));
            };
        };

        namespace literal_character {
            template<typename Character, typename Context>
            struct Closure : LiteralCharacter<Character> {
                using StringIterator = typename Pattern<Character>::StringIterator;
                using Depict = std::function<bool(const Context &, const Character &)>;

                const Context context;
                const Depict depict;

                Closure(Context context, const Depict &depict) :
                        LiteralCharacter<Character>([&](const Character &c) { return this->depict(this->context, c); }),
                        context(std::move(context)), depict(depict) {}

                using Pattern<Character>::match;
            };
        }

        template<typename Character>
        struct Binary : Pattern<Character> {
            using PtrPattern = std::shared_ptr<Pattern<Character>>;

            const PtrPattern first, second;

            Binary(const PtrPattern &first, const PtrPattern &second) : first(first), second(second) {}

            using Pattern<Character>::match;
        };

        namespace binary {
            template<typename Character>
            struct Alternation : Binary<Character> {
                using PtrPattern = std::shared_ptr<Pattern<Character>>;
                using StringIterator = typename Pattern<Character>::StringIterator;
                using PtrRecord = typename Pattern<Character>::PtrRecord;
                using Children = typename Pattern<Character>::Children;

                Alternation(const PtrPattern &first, const PtrPattern &second) : Binary<Character>(first, second) {}

                using Pattern<Character>::match;

                PtrRecord match(const StringIterator &head, const StringIterator &tail) const final {
                    auto m = first->match(head, tail);
                    if (!m->success) m = second->match(head, tail);
                    return m;
                }
            };

            template<typename Character>
            struct Concatenation : Binary<Character> {
                using PtrPattern = std::shared_ptr<Pattern<Character>>;
                using StringIterator = typename Pattern<Character>::StringIterator;
                using PtrRecord = typename Pattern<Character>::PtrRecord;
                using Children = typename Pattern<Character>::Children;

                Concatenation(const PtrPattern &first, const PtrPattern &second) : Binary<Character>(first, second) {}

                using Pattern<Character>::match;

                PtrRecord match(const StringIterator &head, const StringIterator &tail) const final {
                    auto[success, begin, end, children] = *first->match(head, tail);
                    if (success) {
                        auto[success1, begin1, end1, children1] = *second->match(end, tail);
                        return std::make_shared<Record<Character>>(success1, head, end1, children + children1);
                    } else return std::make_shared<Record<Character>>(false, head, end, children);
                }
            };
        }
//
//        template<typename Character>
//        struct Linear : Pattern<Character> {
//            using PtrPattern = typename Pattern<Character>::PtrPattern;
//
//            using List = std::list<PtrPattern>;
//
//            const List list;
//
//            explicit Linear(List list) : list(std::move(list)) {}
//        };
//
//        namespace linear {
//            template<typename Character>
//            struct Alternation : Linear<Character> {
//                using PtrPattern = typename Pattern<Character>::PtrPattern;
//                using StringIterator = typename Pattern<Character>::StringIterator;
//                using Matched = typename Pattern<Character>::Matched;
//                using List = typename Linear<Character>::List;
//
//                explicit Alternation(List list) : Linear<Character>(std::move(list)) {}
//
//                Matched match(const StringIterator &head, const StringIterator &tail) const final {
//                    bool success = false;
//                    auto direct_end = head, greedy_end = head;
//                    std::size_t index = -1;
//                    std::shared_ptr<Record<Character>> value = nullptr;
//
//                    for (auto &&item:this->list) {
//                        auto m = item->match(head, tail);
//                        if (m.second->greedy_end > greedy_end) greedy_end = m.second->greedy_end;
//                        if (m.first) {
//                            success = true;
//                            direct_end = m.second->direct_end;
//                            value = m.second;
//                            break;
//                        }
//                        index++;
//                    }
//                    return {success, std::make_shared<record::LinearSome < Character>>(
//                    head, direct_end, greedy_end,
//                    index, value
//                    )};
//                }
//            };
//
//            template<typename Character>
//            struct Concatenation : Linear<Character> {
//                using PtrPattern = typename Pattern<Character>::PtrPattern;
//                using StringIterator = typename Pattern<Character>::StringIterator;
//                using Matched = typename Pattern<Character>::Matched;
//                using List = typename Linear<Character>::List;
//
//                explicit Concatenation(List list) : Linear<Character>(std::move(list)) {}
//
//                Matched match(const StringIterator &head, const StringIterator &tail) const final {
//                    bool success = true;
//                    auto direct_end = head, greedy_end = head;
//                    typename record::LinearEvery<Character>::Vector vector(this->list.size());
//
//                    std::size_t i = 0;
//                    for (auto &&item:this->list) {
//                        auto m = item->match(direct_end, tail);
//                        direct_end = m.second->direct_end;
//                        if (m.second->greedy_end > greedy_end) greedy_end = m.second->greedy_end;
//                        vector[i++] = m.second;
//                        if (!m.first) {
//                            success = false;
//                            break;
//                        }
//                    }
//                    return {success, std::make_shared<record::LinearEvery < Character>>(
//                    head, direct_end, greedy_end,
//                    std::move(vector)
//                    )};
//                }
//            };
//        }
//
//        template<typename Character>
//        struct Unary : Pattern<Character> {
//            using PtrPattern = typename Pattern<Character>::PtrPattern;
//
//            const PtrPattern value;
//
//            explicit Unary(const PtrPattern &value) : value(value) {}
//        };
//
//        namespace unary {
//            template<typename Character>
//            struct KleeneStar : Unary<Character> {
//                using PtrPattern = typename Pattern<Character>::PtrPattern;
//                using StringIterator = typename Pattern<Character>::StringIterator;
//                using Matched = typename Pattern<Character>::Matched;
//
//                explicit KleeneStar(const PtrPattern &value) : Unary<Character>(value) {}
//
//                Matched match(const StringIterator &head, const StringIterator &tail) const final {
//                    bool success = true;
//                    auto direct_end = head, greedy_end = head;
//                    typename record::Greedy<Character>::List list;
//                    while (true) {
//                        auto m = this->value->match(direct_end, tail);
//                        if (m.second->greedy_end > greedy_end) greedy_end = m.second->greedy_end;
//                        if (m.first && (m.second->direct_end > direct_end)) {
//                            list.emplace_back(m.second);
//                            direct_end = m.second->direct_end;
//                        } else break;
//                    }
//                    return {true, std::make_shared<record::Greedy < Character>>(
//                    head, direct_end, greedy_end,
//                    std::move(list)
//                    )};
//                }
//            };
//
//            template<typename Character>
//            struct Collapse : Unary<Character> {
//                using PtrPattern = typename Pattern<Character>::PtrPattern;
//                using StringIterator = typename Pattern<Character>::StringIterator;
//                using Matched = typename Pattern<Character>::Matched;
//
//                explicit Collapse(const PtrPattern &value) : Unary<Character>(value) {}
//
//                Matched match(const StringIterator &head, const StringIterator &tail) const final {
//                    auto[success, record] = this->value->match(head, tail);
//                    return {success, Record<Character>::collapse(record)};
//                }
//            };
//
//            template<typename Character>
//            struct Mark : Unary<Character> {
//                using PtrPattern = typename Pattern<Character>::PtrPattern;
//                using String = typename CharacterTraits<Character>::String;
//                using StringIterator = typename Pattern<Character>::StringIterator;
//                using Matched = typename Pattern<Character>::Matched;
//
//                const String tag;
//
//                Mark(const PtrPattern &value, String tag) : Unary<Character>(value), tag(std::move(tag)) {}
//
//                Matched match(const StringIterator &head, const StringIterator &tail) const final {
//                    auto[success, record] = this->value->match(head, tail);
//                    return {success, std::make_shared<record::Mark < Character>>(
//                    record->begin, record->direct_end, record->greedy_end,
//                    tag, record
//                    )};
//                }
//            };
//        }
//
//        template<typename Character>
//        struct Placeholder : Pattern<Character> {
//            using PtrPattern = typename Pattern<Character>::PtrPattern;
//            using StringIterator = typename Pattern<Character>::StringIterator;
//            using Matched = typename Pattern<Character>::Matched;
//
//            PtrPattern value = nullptr;
//
//            Matched match(const StringIterator &head, const StringIterator &tail) const final {
//                return this->value ? this->value->match(head, tail) : Matched{false, nullptr};
//            }
//        };
    }
}