#include "reg.h"

namespace reg {
    template<typename Character>
    template<typename Derived>
    inline std::shared_ptr<Derived> Record<Character>::as() const {
        return std::dynamic_pointer_cast<Derived>(const_cast<Record<Character> *>(this)->shared_from_this());
    }

    template<typename Character>
    template<typename Derived>
    inline std::shared_ptr<Derived> Pattern<Character>::as() const {
        return std::dynamic_pointer_cast<Derived>(const_cast<Pattern<Character> *>(this)->shared_from_this());
    }

    namespace pattern {
        template<typename Character>
        inline typename Pattern<Character>::Matched EmptyString<Character>::match(
                const typename Traits<Character>::String::const_iterator &head,
                const typename Traits<Character>::String::const_iterator &
        ) const {
            return {true, std::make_shared<Record<Character>>(head, head, head)};
        }

        template<typename Character>
        typename Pattern<Character>::Matched LiteralCharacter<Character>::match(
                const typename Traits<Character>::String::const_iterator &head,
                const typename Traits<Character>::String::const_iterator &tail
        ) const {
            bool success;
            auto end = head;
            if (head == tail) success = false;
            else {
                success = describe(*head);
                end = std::next(head);
            }
            return {success, std::make_shared<Record<Character>>(head, std::move(end), std::move(end))};
        }

        namespace literal_character {
            template<typename Character, typename Context>
            Closure<Character, Context>::Closure(Context context, Depict depict):
                    LiteralCharacter<Character>([&](const Character &c) { return this->depict(this->context, c); }),
                    context(std::move(context)), depict(std::move(depict)) {}
        }

        namespace linear {
            template<typename Character>
            typename Pattern<Character>::Matched Alternation<Character>::match(
                    const typename Traits<Character>::String::const_iterator &head,
                    const typename Traits<Character>::String::const_iterator &tail
            ) const {
                bool success = false;
                auto direct_end = head, greedy_end = head;
                typename Traits<Character>::String key;
                std::size_t index = 0;
                std::shared_ptr<Record<Character>> record = nullptr;
                for (auto i = this->value.cbegin(); i < this->value.cend();) {
                    auto matched = i->value->match(head, tail);
                    if (matched.success) {
                        success = true;
                        direct_end = matched.record->direct_end;
                        index = std::size_t(i - this->value.cbegin());
                        key = i->key;
                        record = matched.record;
                        i = this->value.cend();
                    } else i++;
                    if (matched.record->greedy_end > greedy_end) greedy_end = matched.record->greedy_end;
                }
                return {success, std::make_shared<record::Some<Character>>(
                        head, std::move(direct_end), std::move(greedy_end),
                        index, std::move(key), std::move(record)
                )};
            }

            template<typename Character>
            typename Pattern<Character>::Matched Concatenation<Character>::match(
                    const typename Traits<Character>::String::const_iterator &head,
                    const typename Traits<Character>::String::const_iterator &tail
            ) const {
                bool success = true;
                auto direct_end = head, greedy_end = head;
                std::unordered_map<typename Traits<Character>::String, std::shared_ptr<Record<Character>>> map;
                std::vector<std::shared_ptr<Record<Character>>> vector(this->value.size(), nullptr);
                for (auto i = this->value.cbegin(); i < this->value.cend();) {
                    auto matched = i->value->match(direct_end, tail);
                    direct_end = matched.record->direct_end;
                    map[i->key] = matched.record;
                    vector[i - this->value.cbegin()] = matched.record;
                    if (!matched.success) {
                        success = false;
                        i = this->value.cend();
                    } else i++;
                    if (matched.record->greedy_end > greedy_end) greedy_end = matched.record->greedy_end;
                }
                return {success, std::make_shared<record::Every<Character >>(
                        head, std::move(direct_end), std::move(greedy_end),
                        std::move(vector), std::move(map)
                )};
            }
        }

        template<typename Character>
        typename Pattern<Character>::Matched KleeneStar<Character>::match(
                const typename Traits<Character>::String::const_iterator &head,
                const typename Traits<Character>::String::const_iterator &tail
        ) const {
            std::list<std::shared_ptr<Record<Character>>> list;
            auto i = head, direct_end = head, greedy_end = head;
            while (true) {
                auto[success, record] = item->match(i, tail);
                if (record->greedy_end > greedy_end) greedy_end = record->greedy_end;
                if (success && (record->direct_end > i)) {
                    list.emplace_back(record);
                    i = record->direct_end;
                } else {
                    direct_end = i;
                    break;
                }
            }
            return {true, std::make_shared<record::Greedy<Character>>(
                    head, std::move(direct_end), std::move(greedy_end),
                    std::move(list)
            )};
        }

        template<typename Character>
        inline typename Pattern<Character>::Matched Operation<Character>::match(
                const typename Traits<Character>::String::const_iterator &head,
                const typename Traits<Character>::String::const_iterator &tail
        ) const {
            auto first_matched = array[0]->match(head, tail);
            if (first_matched.success) {
                bool success;
                auto direct_end = head, greedy_end = first_matched.record->greedy_end;
                auto second_matched = array[1]->match(head, first_matched.record->direct_end);
                if (second_matched.record->greedy_end > greedy_end) greedy_end = second_matched.record->greedy_end;
                if (sign ^ (second_matched.success && second_matched.record->direct_end == direct_end)) {
                    success = false;
                    direct_end = second_matched.record->direct_end;
                } else success = true;
                return {success, std::make_shared<record::Binary<Character>>(
                        head, std::move(direct_end), std::move(greedy_end),
                        typename record::Binary<Character>::Array({first_matched.record, second_matched.record})
                )};
            } else return {false, first_matched.record};
        }

        template<typename Character>
        inline typename Pattern<Character>::Matched Placeholder<Character>::match(
                const typename Traits<Character>::String::const_iterator &head,
                const typename Traits<Character>::String::const_iterator &tail
        ) const {
            return place ? place->match(head, tail) : typename Pattern<Character>::Matched{false, nullptr};
        }

        template<typename Character>
        inline typename Pattern<Character>::Matched Collapsed<Character>::match(
                const typename Traits<Character>::String::const_iterator &head,
                const typename Traits<Character>::String::const_iterator &tail
        ) const {
            auto matched = core->match(head, tail);
            return {matched.success, std::make_shared<Record<Character>>(head, matched.record->direct_end, matched.record->greedy_end)};
        }
    }
}
