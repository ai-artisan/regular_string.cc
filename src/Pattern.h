#pragma once

#include "regular.h"

namespace regular {
    template<typename Character>
    struct Pattern {
        using String = typename CharacterTraits<Character>::String;
        using StringIterator = typename String::const_iterator;
        using PtrRecord = std::shared_ptr<Record < Character>>;

        virtual PtrRecord match(const StringIterator &, const StringIterator &) const = 0;

        PtrRecord match(const String &s) const {
            return match(s.cbegin(), s.cend());
        }
    };

    namespace pattern {
        template<typename Character>
        struct EmptyString : Pattern<Character> {
            using typename Pattern<Character>::StringIterator;
            using typename Pattern<Character>::PtrRecord;

            using Pattern<Character>::match;

            PtrRecord match(const StringIterator &head, const StringIterator &) const final {
                return std::make_shared<Record<Character>>(Record<Character>{true, head, head, {}});
            };
        };

        template<typename Character>
        struct LiteralCharacter : Pattern<Character> {
            using typename Pattern<Character>::StringIterator;
            using typename Pattern<Character>::PtrRecord;
            using Describe = std::function<bool(const Character &)>;

            const Describe describe;

            explicit LiteralCharacter(const Describe &describe) : describe(describe) {}

            using Pattern<Character>::match;

            PtrRecord match(const StringIterator &head, const StringIterator &tail) const final {
                return std::make_shared<Record<Character>>(
                        head == tail
                        ? Record<Character>{false, head, head, {}}
                        : Record<Character>{describe(*head), head, std::next(head), {}}
                );
            };
        };

        namespace literal_character {
            template<typename Character, typename Context>
            struct Closure : LiteralCharacter<Character> {
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
                using typename Pattern<Character>::StringIterator;
                using typename Pattern<Character>::PtrRecord;
                using typename Binary<Character>::PtrPattern;

                Alternation(const PtrPattern &first, const PtrPattern &second) : Binary<Character>(first, second) {}

                using Pattern<Character>::match;

                PtrRecord match(const StringIterator &head, const StringIterator &tail) const final {
                    auto r = first->match(head, tail);
                    if (!r->success) r = second->match(head, tail);
                    return r;
                }
            };

            template<typename Character>
            struct Concatenation : Binary<Character> {
                using typename Pattern<Character>::StringIterator;
                using typename Pattern<Character>::PtrRecord;
                using typename Binary<Character>::PtrPattern;

                Concatenation(const PtrPattern &first, const PtrPattern &second) : Binary<Character>(first, second) {}

                using Pattern<Character>::match;

                PtrRecord match(const StringIterator &head, const StringIterator &tail) const final {
                    auto r = first->match(head, tail);
                    if (r->success) {
                        auto r1 = second->match(r->end, tail);
                        r1->begin = r->begin;
                        r1->children.splice(r1->children.begin(), r->children);
                        return r1;
                    } else return r;
                }
            };
        }

        template<typename Character>
        struct Linear : Pattern<Character> {
            using PtrPattern = std::shared_ptr<Pattern<Character>>;

            using List = std::list<PtrPattern>;

            const List list;

            explicit Linear(List list) : list(std::move(list)) {}
        };

        namespace linear {
            template<typename Character>
            struct Alternation : Linear<Character> {
                using typename Pattern<Character>::StringIterator;
                using typename Pattern<Character>::PtrRecord;
                using typename Linear<Character>::List;

                explicit Alternation(List list) : Linear<Character>(std::move(list)) {}

                using Pattern<Character>::match;

                PtrRecord match(const StringIterator &head, const StringIterator &tail) const final {
                    auto r = std::make_shared<Record<Character>>();
                    r->success = false;
                    r->begin = r->end = head;
                    for (auto &&item:list) {
                        r = item->match(head, tail);
                        if (r->success) break;
                    }
                    return r;
                }
            };

            template<typename Character>
            struct Concatenation : Linear<Character> {
                using typename Pattern<Character>::StringIterator;
                using typename Pattern<Character>::PtrRecord;
                using typename Linear<Character>::List;

                explicit Concatenation(List list) : Linear<Character>(std::move(list)) {}

                using Pattern<Character>::match;

                PtrRecord match(const StringIterator &head, const StringIterator &tail) const final {
                    auto r = std::make_shared<Record<Character>>();
                    r->success = true;
                    r->begin = r->end = head;
                    for (auto &&item:list) {
                        auto r1 = item->match(r->end, tail);
                        r->end = r1->end;
                        r->children.splice(r->children.end(), r1->children);
                        if (!r1->success) {
                            r->success = false;
                            break;
                        }
                    }
                    return r;
                }
            };
        }

        template<typename Character>
        struct Unary : Pattern<Character> {
            using PtrPattern = std::shared_ptr<Pattern<Character>>;

            const PtrPattern value;

            explicit Unary(const PtrPattern &value) : value(value) {}
        };

        namespace unary {
            template<typename Character>
            struct KleeneStar : Unary<Character> {
                using typename Pattern<Character>::StringIterator;
                using typename Pattern<Character>::PtrRecord;
                using typename Unary<Character>::PtrPattern;

                explicit KleeneStar(const PtrPattern &value) : Unary<Character>(value) {}

                using Pattern<Character>::match;

                PtrRecord match(const StringIterator &head, const StringIterator &tail) const final {
                    auto r = std::make_shared<Record<Character>>();
                    r->success = true;
                    r->begin = r->end = head;
                    while (true) {
                        auto r1 = value->match(r->end, tail);
                        if (r1->success && (r1->end > r->end)) {
                            r->end = r1->end;
                            r->children.splice(r->children.end(), r1->children);
                        } else break;
                    }
                    return r;
                }
            };

            template<typename Character>
            struct Mark : Unary<Character> {
                using typename Pattern<Character>::String;
                using typename Pattern<Character>::StringIterator;
                using typename Pattern<Character>::PtrRecord;
                using typename Unary<Character>::PtrPattern;

                const String tag;

                Mark(const PtrPattern &value, String tag) : Unary<Character>(value), tag(std::move(tag)) {}

                using Pattern<Character>::match;

                PtrRecord match(const StringIterator &head, const StringIterator &tail) const final {
                    auto r = value->match(head, tail);
                    return std::make_shared<Record<Character>>(Record<Character>{
                            r->success, r->begin, r->end, {{tag, r}}
                    });
                }
            };
        }

        template<typename Character>
        struct Placeholder : Pattern<Character> {
            using typename Pattern<Character>::StringIterator;
            using typename Pattern<Character>::PtrRecord;
            using PtrPattern = std::shared_ptr<Pattern<Character>>;

            PtrPattern value = nullptr;

            using Pattern<Character>::match;

            PtrRecord match(const StringIterator &head, const StringIterator &tail) const final {
                return value ? value->match(head, tail) : nullptr;
            }
        };
    }
}