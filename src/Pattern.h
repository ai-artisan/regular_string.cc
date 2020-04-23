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
            return std::dynamic_pointer_cast<Derived>(const_cast<Pattern *>(this)->shared_from_this());
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
                    auto m = this->first->match(head, tail);
                    auto greedy_end = m.second->greedy_end;
                    if (m.first) index = false;
                    else {
                        m = this->second->match(head, tail);
                        index = true;
                        if (m.second->greedy_end > greedy_end) greedy_end = m.second->greedy_end;
                    }
                    return {m.first, std::make_shared<record::Some<Character>>(
                            head, m.second->direct_end, greedy_end,
                            index, m.second
                    )};
                }
            };

            template<typename Character>
            struct Concatenation : Binary<Character> {
                using PtrPattern = typename Pattern<Character>::PtrPattern;
                using StringIterator = typename Pattern<Character>::StringIterator;
                using Matched = typename Pattern<Character>::Matched;

                Concatenation(const PtrPattern &first, const PtrPattern &second) : Binary<Character>(first, second) {}

                Matched match(const StringIterator &head, const StringIterator &tail) const final {
                    auto m = this->first->match(head, tail);
                    auto success = m.first;
                    decltype(m.second) first_record = m.second, second_record;
                    auto greedy_end = first_record->greedy_end;
                    if (success) {
                        m = this->second->match(m.second->direct_end, tail);
                        success = success && m.first;
                        second_record = m.second;
                        if (second_record->greedy_end > greedy_end) greedy_end = second_record->greedy_end;
                    } else second_record = nullptr;
                    return {success, std::make_shared<record::Every<Character>>(
                            head, m.second->direct_end, greedy_end,
                            first_record, second_record
                    )};
                }
            };

            template<typename Character>
            struct Filter : Binary<Character> {
                using PtrPattern = typename Pattern<Character>::PtrPattern;
                using StringIterator = typename Pattern<Character>::StringIterator;
                using Matched = typename Pattern<Character>::Matched;

                const bool sign;

                Filter(const PtrPattern &first, const PtrPattern &second, const bool &sign) :
                        Binary<Character>(first, second), sign(sign) {}

                Matched match(const StringIterator &head, const StringIterator &tail) const final {
                    auto m0 = this->first->match(head, tail);
                    if (m0.first) {
                        auto m1 = this->second->match(head, m0.second->direct_end);
                        return {
                                !((m1.first && m1.second->direct_end == m0.second->direct_end) ^ this->sign),
                                std::make_shared<record::Every<Character>>(
                                        head, m0.second->direct_end, std::max(m0.second->greedy_end, m1.second->greedy_end),
                                        m0.second, m1.second
                                )
                        };
                    } else
                        return {false, std::make_shared<record::Every<Character>>(
                                head, m0.second->direct_end, m0.second->greedy_end,
                                m0.second, nullptr
                        )};
                }
            };
        }

        template<typename Character>
        struct Linear : Pattern<Character> {
            using PtrPattern = typename Pattern<Character>::PtrPattern;

            using List = std::list<PtrPattern>;

            const List list;

            explicit Linear(List list) : list(std::move(list)) {}
        };

        namespace linear {
            template<typename Character>
            struct Alternation : Linear<Character> {
                using PtrPattern = typename Pattern<Character>::PtrPattern;
                using StringIterator = typename Pattern<Character>::StringIterator;
                using Matched = typename Pattern<Character>::Matched;
                using List = typename Linear<Character>::List;

                explicit Alternation(List list) : Linear<Character>(std::move(list)) {}

                Matched match(const StringIterator &head, const StringIterator &tail) const final {
                    bool success = false;
                    auto direct_end = head, greedy_end = head;
                    std::size_t index = -1;
                    std::shared_ptr<Record<Character>> value = nullptr;

                    for (auto &&item:this->list) {
                        auto m = item->match(head, tail);
                        if (m.second->greedy_end > greedy_end) greedy_end = m.second->greedy_end;
                        if (m.first) {
                            success = true;
                            direct_end = m.second->direct_end;
                            value = m.second;
                            break;
                        }
                        index++;
                    }
                    return {success, std::make_shared<record::LinearSome<Character>>(
                            head, direct_end, greedy_end,
                            index, value
                    )};
                }
            };

            template<typename Character>
            struct Concatenation : Linear<Character> {
                using PtrPattern = typename Pattern<Character>::PtrPattern;
                using StringIterator = typename Pattern<Character>::StringIterator;
                using Matched = typename Pattern<Character>::Matched;
                using List = typename Linear<Character>::List;

                explicit Concatenation(List list) : Linear<Character>(std::move(list)) {}

                Matched match(const StringIterator &head, const StringIterator &tail) const final {
                    bool success = true;
                    auto direct_end = head, greedy_end = head;
                    typename record::LinearEvery<Character>::Vector vector(this->list.size());

                    std::size_t i = 0;
                    for (auto &&item:this->list) {
                        auto m = item->match(direct_end, tail);
                        direct_end = m.second->direct_end;
                        if (m.second->greedy_end > greedy_end) greedy_end = m.second->greedy_end;
                        vector[i++] = m.second;
                        if (!m.first) {
                            success = false;
                            break;
                        }
                    }
                    return {success, std::make_shared<record::LinearEvery<Character>>(
                            head, direct_end, greedy_end,
                            std::move(vector)
                    )};
                }
            };
        }

        template<typename Character>
        struct Unary : Pattern<Character> {
            using PtrPattern = typename Pattern<Character>::PtrPattern;

            const PtrPattern value;

            explicit Unary(const PtrPattern &value) : value(value) {}
        };

        namespace unary {
            template<typename Character>
            struct KleeneStar : Unary<Character> {
                using PtrPattern = typename Pattern<Character>::PtrPattern;
                using StringIterator = typename Pattern<Character>::StringIterator;
                using Matched = typename Pattern<Character>::Matched;

                explicit KleeneStar(const PtrPattern &value) : Unary<Character>(value) {}

                Matched match(const StringIterator &head, const StringIterator &tail) const final {
                    bool success = true;
                    auto direct_end = head, greedy_end = head;
                    typename record::Greedy<Character>::List list;
                    while (true) {
                        auto m = this->value->match(direct_end, tail);
                        if (m.second->greedy_end > greedy_end) greedy_end = m.second->greedy_end;
                        if (m.first && (m.second->direct_end > direct_end)) {
                            list.emplace_back(m.second);
                            direct_end = m.second->direct_end;
                        } else break;
                    }
                    return {true, std::make_shared<record::Greedy<Character>>(
                            head, direct_end, greedy_end,
                            std::move(list)
                    )};
                }
            };

            template<typename Character>
            struct Collapse : Unary<Character> {
                using PtrPattern = typename Pattern<Character>::PtrPattern;
                using StringIterator = typename Pattern<Character>::StringIterator;
                using Matched = typename Pattern<Character>::Matched;

                explicit Collapse(const PtrPattern &value) : Unary<Character>(value) {}

                Matched match(const StringIterator &head, const StringIterator &tail) const final {
                    auto[success, record] = this->value->match(head, tail);
                    return {success, Record<Character>::collapse(record)};
                }
            };

            template<typename Character>
            struct Mark : Unary<Character> {
                using PtrPattern = typename Pattern<Character>::PtrPattern;
                using String = typename CharacterTraits<Character>::String;
                using StringIterator = typename Pattern<Character>::StringIterator;
                using Matched = typename Pattern<Character>::Matched;

                const String tag;

                Mark(const PtrPattern &value, String tag) : Unary<Character>(value), tag(std::move(tag)) {}

                Matched match(const StringIterator &head, const StringIterator &tail) const final {
                    auto[success, record] = this->value->match(head, tail);
                    return {success, std::make_shared<record::Mark<Character>>(
                            record->begin, record->direct_end, record->greedy_end,
                            tag, record
                    )};
                }
            };
        }

        template<typename Character>
        struct Placeholder : Pattern<Character> {
            using PtrPattern = typename Pattern<Character>::PtrPattern;
            using StringIterator = typename Pattern<Character>::StringIterator;
            using Matched = typename Pattern<Character>::Matched;

            PtrPattern value = nullptr;

            Matched match(const StringIterator &head, const StringIterator &tail) const final {
                return this->value ? this->value->match(head, tail) : Matched{false, nullptr};
            }
        };
    }
}