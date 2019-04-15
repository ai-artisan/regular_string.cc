#include "regular.h"

namespace regular {
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
            return {true, ({
                auto r = std::make_shared<Record<Character>>();
                r->begin = begin;
                r->end = begin;
                r;
            })};
        }

        template<typename Character>
        typename Pattern<Character>::Matched Singleton<Character>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            bool success;
            auto end1 = (begin == end ? ({
                success = false;
                begin;
            }) : ({
                success = describe(*begin);
                std::next(begin);
            }));
            return {success, ({
                auto r = std::make_shared<Record<Character>>();
                r->begin = begin;
                r->end = end1;
                r;
            })};
        }

        namespace singleton {
            template<typename Character, typename Context>
            Closure<Character, Context>::Closure(Context &&context, const TYPE(depict) &depict):
                    Singleton<Character>([&](const Character &c) { return this->depict(this->context, c); }), context(std::move(context)), depict(depict) {}
        }

        namespace linear {
            template<typename Character>
            typename Pattern<Character>::Matched Union<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                bool success = false;
                auto end1 = begin;
                typename Traits<Character>::String key(0, 0);
                std::shared_ptr<Record<Character>> record = nullptr;
                for (auto i = this->linear.cbegin(); i != this->linear.cend(); ({
                    auto matched = i->value->match(begin, end);
                    if (matched.success) {
                        success = true;
                        end1 = matched.record->end;
                        key = i->key;
                        record = matched.record;
                        i = this->linear.cend();
                    } else i++;
                }));
                return {success, ({
                    auto r = std::make_shared<record::LinearSome<Character>>();
                    r->begin = begin;
                    r->end = end1;
                    r->key = std::move(key);
                    r->value = record;
                    r;
                })};
            }

            template<typename Character>
            typename Pattern<Character>::Matched Intersection<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                bool success = true;
                auto end1 = begin;
                std::unordered_map<typename Traits<Character>::String, std::shared_ptr<Record<Character>>> map;
                std::list<std::shared_ptr<Record<Character>>> list;
                if (!this->linear.empty()) {
                    auto matched = this->linear.front().value->match(begin, end);
                    map[this->linear.front().key] = matched.record;
                    list.emplace_back(matched.record);
                    if (matched.success) {
                        end1 = matched.record->end;
                        for (auto i = std::next(this->linear.cbegin()); i != this->linear.cend(); ({
                            auto matched1 = i->value->match(begin, end1);
                            map[i->key] = matched1.record;
                            list.emplace_back(matched1.record);
                            if (matched1.success && matched1.record->end == end1) i++;
                            else {
                                success = false;
                                end1 = matched1.record->end;
                                i = this->linear.cend();
                            }
                        }));
                    } else success = false;
                }
                return {success, ({
                    auto r = std::make_shared<record::list::LinearEvery<Character>>();
                    r->begin = begin;
                    r->end = end1;
                    r->map = std::move(map);
                    r->list = std::move(list);
                    r;
                })};
            }

            template<typename Character>
            typename Pattern<Character>::Matched Difference<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                bool success = false;
                auto end1 = begin;
                typename Traits<Character>::String key(0, 0);
                std::shared_ptr<Record<Character>> record = nullptr;

                if (!this->linear.empty()) {
                    auto matched = this->linear.front().value->match(begin, end);
                    end1 = matched.record->end;
                    if (matched.success) {
                        for (auto i = this->linear.crbegin(); i != std::prev(this->linear.crend()); ({
                            auto matched1 = i->value->match(begin, end1);
                            success = matched1.success && matched1.record->end == end1 && !success;
                            if (success) {
                                key = i->key;
                                record = matched1.record;
                            }
                            i++;
                        }));
                        success ^= true;
                        if (success) {
                            key = this->linear.front().key;
                            record = matched.record;
                        }
                    } else {
                        key = this->linear.front().key;
                        record = matched.record;
                    }
                }

                return {!(success xor sign), ({
                    auto r = std::make_shared<record::LinearSome<Character>>();
                    r->begin = begin;
                    r->end = end1;
                    r->key = std::move(key);
                    r->value = record;
                    r;
                })};
            }

            template<typename Character>
            typename Pattern<Character>::Matched Concatenation<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                bool success = true;
                auto end1 = begin;
                std::unordered_map<typename Traits<Character>::String, std::shared_ptr<Record<Character>>> map;
                std::list<std::shared_ptr<Record<Character>>> list;
                for (auto i = this->linear.cbegin(); i != this->linear.cend(); ({
                    auto matched = i->value->match(end1, end);
                    end1 = matched.record->end;
                    map[i->key] = matched.record;
                    list.emplace_back(matched.record);
                    if (!matched.success) {
                        success = false;
                        i = this->linear.cend();
                    } else i++;
                }));
                return {success, ({
                    auto r = std::make_shared<record::list::LinearEvery<Character>>();
                    r->begin = begin;
                    r->end = end1;
                    r->map = std::move(map);
                    r->list = std::move(list);
                    r;
                })};
            }
        }

        template<typename Character>
        typename Pattern<Character>::Matched KleeneClosure<Character>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            std::list<std::shared_ptr<Record<Character>>> list;
            auto i = begin, end1 = end;
            while (({
                auto[success, record] = item->match(i, end);
                success && (record->end > i) ? ({
                    list.emplace_back(record);
                    i = record->end;
                    true;
                }) : ({
                    end1 = i;
                    false;
                });
            }));
            return {true, ({
                auto r = std::make_shared<record::Kleene<Character>>();
                r->begin = begin;
                r->end = end1;
                r->list = std::move(list);
                r;
            })};
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
            return {matched.success, ({
                auto r = std::make_shared<Record<Character>>();
                r->begin = begin;
                r->end = matched.record->end;
                r;
            })};
        }

        template<typename Character>
        inline typename Pattern<Character>::Matched Custom<Character>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            auto matched = base->match(begin, end);
            return {matched.success, process(matched)};
        }
    }
}

#undef TYPE
