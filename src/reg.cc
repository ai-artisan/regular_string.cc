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
        inline typename Pattern<Character>::Matched Empty<Character>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            auto r = std::make_shared<Record<Character>>();
            r->begin = begin;
            r->end = begin;
            return {true, r};
        }

        template<typename Character>
        typename Pattern<Character>::Matched Singleton<Character>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            bool success;
            typename Traits<Character>::String::const_iterator end1;
            if (begin == end) {
                success = false;
                end1 = begin;
            } else {
                success = describe(*begin);
                end1 = std::next(begin);
            }
            auto r = std::make_shared<Record<Character>>();
            r->begin = begin;
            r->end = end1;
            return {success, r};
        }

        namespace singleton {
            template<typename Character, typename Context>
            Closure<Character, Context>::Closure(Context &&context, const TYPE(depict) &depict):
                    Singleton<Character>([&](const Character &c) { return this->depict(this->context, c); }),
                    context(std::move(context)), depict(depict) {}
        }

        namespace linear {
            template<typename Character>
            typename Pattern<Character>::Matched Union<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                bool success = false;
                auto end1 = begin;
                typename Traits<Character>::String key;
                std::size_t index = 0;
                std::shared_ptr<Record<Character>> record = nullptr;
                for (auto i = this->linear.cbegin(); i < this->linear.cend();) {
                    auto matched = i->value->match(begin, end);
                    if (matched.success) {
                        success = true;
                        end1 = matched.record->end;
                        index = std::size_t(i - this->linear.cbegin());
                        key = i->key;
                        record = matched.record;
                        i = this->linear.cend();
                    } else {
                        if (matched.record->end > end1) end1 = matched.record->end;
                        i++;
                    }
                }
                auto r = std::make_shared<record::LinearSome<Character>>();
                r->begin = begin;
                r->end = end1;
                r->index = index;
                r->key = std::move(key);
                r->value = record;
                return {success, r};
            }

            template<typename Character>
            typename Pattern<Character>::Matched Intersection<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                bool success = true;
                auto end1 = begin;
                std::unordered_map<typename Traits<Character>::String, std::shared_ptr<Record<Character>>> map;
                std::vector<std::shared_ptr<Record<Character>>> vector(this->linear.size(), nullptr);
                for (auto i = this->linear.cbegin(); i < this->linear.cend();) {
                    auto matched = i->value->match(begin, end);
                    map[i->key] = matched.record;
                    vector[i - this->linear.cbegin()] = matched.record;
                    if (matched.success) i++;
                    else {
                        success = false;
                        end1 = matched.record->end;
                        i = this->linear.cend();
                    }
                }
                auto r = std::make_shared<record::LinearEvery<Character>>();
                r->begin = begin;
                r->end = end1;
                r->map = std::move(map);
                r->vector = std::move(vector);
                return {success, r};
            }

            template<typename Character>
            typename Pattern<Character>::Matched Difference<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                bool success = false;
                auto end1 = begin;
                std::size_t index = 0;
                typename Traits<Character>::String key;
                std::shared_ptr<Record<Character>> record = nullptr;
                for (auto i = this->linear.crbegin(); i < this->linear.crend(); i++) {
                    auto matched = i->value->match(begin, end);
                    success = matched.success && !success;
                    if (success) {
                        end1 = matched.record->end;
                        index = std::size_t(this->linear.crend() - i - 1);
                        key = i->key;
                        record = matched.record;
                    }
                }
                auto r = std::make_shared<record::LinearSome<Character>>();
                r->begin = begin;
                r->end = end1;
                r->index = index;
                r->key = std::move(key);
                r->value = record;
                return {!(success ^ sign), r};
            }

            template<typename Character>
            typename Pattern<Character>::Matched Concatenation<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                bool success = true;
                auto end1 = begin;
                std::unordered_map<typename Traits<Character>::String, std::shared_ptr<Record<Character>>> map;
                std::vector<std::shared_ptr<Record<Character>>> vector(this->linear.size(), nullptr);
                for (auto i = this->linear.cbegin(); i < this->linear.cend();) {
                    auto matched = i->value->match(end1, end);
                    end1 = matched.record->end;
                    map[i->key] = matched.record;
                    vector[i - this->linear.cbegin()] = matched.record;
                    if (!matched.success) {
                        success = false;
                        i = this->linear.cend();
                    } else i++;
                }
                auto r = std::make_shared<record::LinearEvery<Character >>();
                r->begin = begin;
                r->end = end1;
                r->map = std::move(map);
                r->vector = std::move(vector);
                return {success, r};
            }
        }

        template<typename Character>
        typename Pattern<Character>::Matched KleeneClosure<Character>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            std::list<std::shared_ptr<Record<Character>>> list;
            auto i = begin, end1 = end;
            while (true) {
                auto[success, record] = item->match(i, end);
                if (success && (record->end > i)) {
                    list.emplace_back(record);
                    i = record->end;
                } else {
                    end1 = i;
                    break;
                }
            }
            auto r = std::make_shared<record::Kleene<Character>>();
            r->begin = begin;
            r->end = end1;
            r->list = std::move(list);
            return {true, r};
        }

        template<typename Character>
        inline typename Pattern<Character>::Matched Placeholder<Character>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            return place ? place->match(begin, end) : typename Pattern<Character>::Matched{false, nullptr};
        }

        template<typename Character>
        inline typename Pattern<Character>::Matched Collapsed<Character>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            auto matched = core->match(begin, end);
            auto r = std::make_shared<Record<Character>>();
            r->begin = begin;
            r->end = matched.record->end;
            return {matched.success, r};
        }
    }
}

#undef TYPE
