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
        typename Pattern<Character>::Matched inline Empty<Character>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            return {true, std::make_shared<Record<Character>>(begin)};
        }

        template<typename Character>
        typename Pattern<Character>::Matched Singleton<Character>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            if (begin != end && describe(*begin)) return {true, std::make_shared<Record<Character>>(std::next(begin))};
            else return {false, std::make_shared<Record<Character>>(begin)};
        }

        namespace singleton {
            template<typename Character, typename Context>
            Closure<Character, Context>::Closure(Context &&context, const decltype(depict) &depict):
                    Singleton<Character>([&](const Character &c) { return this->depict(this->context, c); }), context(std::move(context)), depict(depict) {}
        }

        namespace binary {
            template<typename Character>
            typename Pattern<Character>::Matched Union<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                bool index = false;
                auto matched = this->binary[0]->match(begin, end);
                if (!matched.success) {
                    index = true;
                    matched = this->binary[1]->match(begin, end);
                }
                return {matched.success, std::make_shared<record::BinarySome<Character>>(matched.record->end, index, matched.record)};
            }

            template<typename Character>
            typename Pattern<Character>::Matched Intersection<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                bool success = true;
                std::array<std::shared_ptr<Record<Character>>, 2> every = {nullptr, nullptr};
                auto matched = this->binary[0]->match(begin, end);
                every[0] = matched.record;
                if (matched.success) {
                    matched = this->binary[1]->match(begin, every[0]->end);
                    every[1] = matched.record;
                    if (matched.record->end != every[0]->end) success = false;
                } else success = false;
                return {matched.success, std::make_shared<record::BinaryEvery<Character>>(every[1]->end, std::move(every))};
            }

            template<typename Character>
            typename Pattern<Character>::Matched Difference<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                bool index = false;
                auto matched = this->binary[0]->match(begin, end);
                if (matched.success) {
                    index = true;
                    auto matched1 = this->binary[1]->match(begin, matched.record->end);
                    if (matched1.success && matched1.record->end == matched.record->end) matched = {false, matched1.record};
                }
                return {matched.success, std::make_shared<record::BinarySome<Character>>(matched.record->end, index, matched.record)};
            }

            template<typename Character>
            typename Pattern<Character>::Matched Concatenation<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                std::array<std::shared_ptr<Record<Character>>, 2> every = {nullptr, nullptr};
                auto matched = this->binary[0]->match(begin, end);
                every[0] = matched.record;
                if (matched.success) {
                    matched = this->binary[1]->match(matched.record->end, end);
                    every[1] = matched.record;
                }
                return {matched.success, std::make_shared<record::BinaryEvery<Character>>(every[1]->end, std::move(every))};
            }
        }

        namespace linear {
            template<typename Character>
            typename Pattern<Character>::Matched Union<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                bool success = false;
                auto end1 = begin;
                typename Traits<Character>::String key = "";
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
                return {success, std::make_shared<record::LinearSome<Character>>(end1, std::move(key), record)};
            }

            template<typename Character>
            typename Pattern<Character>::Matched Intersection<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                bool success = true;
                auto end1 = begin;
                std::unordered_map<typename Traits<Character>::String, std::shared_ptr<Record<Character>>> every;
                if (!this->linear.empty()) {
                    auto matched = this->linear.front().value->match(begin, end);
                    every[this->linear.front().key] = matched.record;
                    if (matched.success) {
                        end1 = matched.record->end;
                        for (auto i = std::next(this->linear.cbegin()); i != this->linear.cend(); ({
                            auto matched1 = i->value->match(begin, end1);
                            every[i->key] = matched1.record;
                            if (matched1.success && matched1.record->end == end1) i++;
                            else {
                                success = false;
                                end1 = matched1.record->end;
                                i = this->linear.cend();
                            }
                        }));
                    } else success = false;
                }
                return {success, std::make_shared<record::LinearEvery<Character>>(end1, std::move(every))};
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

                return {success, std::make_shared<record::LinearSome<Character>>(end1, std::move(key), record)};
            }

            template<typename Character>
            typename Pattern<Character>::Matched Concatenation<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                bool success = true;
                auto end1 = begin;
                std::unordered_map<typename Traits<Character>::String, std::shared_ptr<Record<Character>>> every;
                for (auto i = this->linear.cbegin(); i != this->linear.cend(); ({
                    auto matched = i->value->match(end1, end);
                    end1 = matched.record->end;
                    every[i->key] = matched.record;
                    if (!matched.success) {
                        success = false;
                        i = this->linear.cend();
                    } else i++;
                }));
                return {success, std::make_shared<record::LinearEvery<Character>>(end1, std::move(every))};
            }
        }

        template<typename Character>
        typename Pattern<Character>::Matched KleeneClosure<Character>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            std::list<std::shared_ptr<Record<Character>>> list;
            auto i = begin, j = end;
            while (({
                auto[success, record] = item->match(i, end);
                success && (record->end > i) ? ({
                    list.emplace_back(record);
                    i = record->end;
                    true;
                }) : ({
                    j = i;
                    false;
                });
            }));
            return {true, std::make_shared<record::KleeneClosure<Character>>(j, std::move(list))};
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
            return {matched.success, std::make_shared<Record<Character>>(matched.record->end)};
        }
    }


    template<typename Character>
    inline std::shared_ptr<pattern::Empty<Character>> shortcut<Character>::pe() {
        return std::make_shared<pattern::Empty<Character>>();
    }

    template<typename Character>
    inline std::shared_ptr<pattern::Singleton<Character>> shortcut<Character>::ps(const std::function<bool(const Character &)> &describe) {
        return std::make_shared<pattern::Singleton<Character>>(describe);
    }

    template<typename Character>
    template<typename Context>
    inline std::shared_ptr<pattern::singleton::Closure<Character, Context>> shortcut<Character>::ps(Context &&context, const decltype(pattern::singleton::Closure<Character, Context>::depict) &depict) {
        return std::make_shared<pattern::singleton::Closure<Character, Context>>(std::forward<Context>(context), depict);
    }

    template<typename Character>
    inline std::shared_ptr<pattern::Singleton<Character>> shortcut<Character>::psa() {
        return std::make_shared<pattern::Singleton<Character>>([](const Character &) {
            return true;
        });
    }

    template<typename Character>
    inline std::shared_ptr<pattern::singleton::Closure<Character, Character>> shortcut<Character>::psc(const Character &c0) {
        return std::make_shared<pattern::singleton::Closure<Character, Character>>(Character(c0), [](const Character &c0, const Character &c) -> bool {
            return c == c0;
        });
    }

    template<typename Character>
    std::shared_ptr<pattern::singleton::Closure<Character, typename Traits<Character>::String>> shortcut<Character>::pss(typename Traits<Character>::String &&s) {
        return std::make_shared<pattern::singleton::Closure<Character, typename Traits<Character>::String>>(std::move(s), [&](const typename Traits<Character>::String &s, const Character &c) -> bool {
            for (auto i = s.cbegin(); i != s.cend(); ({
                if (c == *i) return true;
                i++;
            }));
            return false;
        });
    }

    template<typename Character>
    inline std::shared_ptr<pattern::singleton::Closure<Character, std::array<Character, 2>>> shortcut<Character>::psr(const Character &inf, const Character &sup) {
        return std::make_shared<pattern::singleton::Closure<Character, std::array<Character, 2>>>(std::array{inf, sup}, [&](const std::array<Character, 2> &interval, const Character &c) -> bool {
            return interval[0] <= c && c <= interval[1];
        });
    }

    template<typename Character>
    std::shared_ptr<pattern::singleton::Closure<Character,
            std::list<std::shared_ptr<pattern::Singleton<Character>>>
    >> shortcut<Character>::psu(std::list<std::shared_ptr<pattern::Singleton<Character>>> &&list) {
        return std::make_shared<pattern::singleton::Closure<
                Character,
                std::list<std::shared_ptr<pattern::Singleton<Character>>>
        >>(std::move(list), [&](const std::list<std::shared_ptr<pattern::Singleton<Character>>> &list, const Character &c) -> bool {
            for (auto i = list.cbegin(); i != list.cend(); ({
                if ((*i)->describe(c)) return true;
                i++;
            }));
            return false;
        });
    }

    template<typename Character>
    std::shared_ptr<pattern::singleton::Closure<
            Character,
            std::list<std::shared_ptr<pattern::Singleton<Character>>>
    >> shortcut<Character>::psi(std::list<std::shared_ptr<pattern::Singleton<Character>>> &&list) {
        return std::make_shared<pattern::singleton::Closure<
                Character,
                std::list<std::shared_ptr<pattern::Singleton<Character>>>
        >>(std::move(list), [&](const std::list<std::shared_ptr<pattern::Singleton<Character>>> &list, const Character &c) -> bool {
            for (auto i = list.cbegin(); i != list.cend(); ({
                if (!(*i)->describe(c)) return false;
                i++;
            }));
            return true;
        });
    }

    template<typename Character>
    std::shared_ptr<pattern::singleton::Closure<
            Character,
            std::list<std::shared_ptr<pattern::Singleton<Character>>>
    >> shortcut<Character>::psd(std::list<std::shared_ptr<pattern::Singleton<Character>>> &&list) {
        return std::make_shared<pattern::singleton::Closure<
                Character,
                std::list<std::shared_ptr<pattern::Singleton<Character>>>
        >>(std::move(list), [&](const std::list<std::shared_ptr<pattern::Singleton<Character>>> &list, const Character &c) -> bool {
            bool b = false;
            for (auto i = list.crbegin(); i != list.crend(); ({
                b = (*i)->describe(c) && !b;
                i++;
            }));
            return b;
        });
    }

    template<typename Character>
    inline std::shared_ptr<pattern::binary::Union<Character>> shortcut<Character>::pbu(const std::shared_ptr<regular::Pattern<Character>> &p0, const std::shared_ptr<regular::Pattern<Character>> &p1) {
        return std::make_shared<pattern::binary::Union<Character>>(std::array{p0, p1});
    }

    template<typename Character>
    inline std::shared_ptr<pattern::binary::Intersection<Character>> shortcut<Character>::pbi(const std::shared_ptr<regular::Pattern<Character>> &p0, const std::shared_ptr<regular::Pattern<Character>> &p1) {
        return std::make_shared<pattern::binary::Intersection<Character>>(std::array{p0, p1});
    }

    template<typename Character>
    inline std::shared_ptr<pattern::binary::Difference<Character>> shortcut<Character>::pbd(const std::shared_ptr<regular::Pattern<Character>> &p0, const std::shared_ptr<regular::Pattern<Character>> &p1) {
        return std::make_shared<pattern::binary::Difference<Character>>(std::array{p0, p1});
    }

    template<typename Character>
    inline std::shared_ptr<pattern::binary::Concatenation<Character>> shortcut<Character>::pbc(const std::shared_ptr<regular::Pattern<Character>> &p0, const std::shared_ptr<regular::Pattern<Character>> &p1) {
        return std::make_shared<pattern::binary::Concatenation<Character>>(std::array{p0, p1});
    }

    template<typename Character>
    std::shared_ptr<typename shortcut<Character>::pt> shortcut<Character>::pbc(const typename Traits<Character>::String &s) {
        std::shared_ptr<pt> p = pe();
        for (auto i = s.crbegin(); i != s.crend(); ({
            std::array<std::shared_ptr<pt>, 2> binary = {psc(*i), p};
            p = std::make_shared<pbct>(std::move(binary));
            i++;
        }));
        return p;
    }

    template<typename Character>
    inline std::shared_ptr<pattern::linear::Union<Character>> shortcut<Character>::plu(std::list<typename pattern::Linear<Character>::Item> &&list) {
        return std::make_shared<pattern::linear::Union<Character>>(std::move(list));
    }

    template<typename Character>
    inline std::shared_ptr<pattern::linear::Intersection<Character>> shortcut<Character>::pli(std::list<typename pattern::Linear<Character>::Item> &&list) {
        return std::make_shared<pattern::linear::Intersection<Character>>(std::move(list));
    }

    template<typename Character>
    inline std::shared_ptr<pattern::linear::Difference<Character>> shortcut<Character>::pld(std::list<typename pattern::Linear<Character>::Item> &&list) {
        return std::make_shared<pattern::linear::Difference<Character>>(std::move(list));
    }

    template<typename Character>
    inline std::shared_ptr<pattern::linear::Concatenation<Character>> shortcut<Character>::plc(std::list<typename pattern::Linear<Character>::Item> &&list) {
        return std::make_shared<pattern::linear::Concatenation<Character>>(std::move(list));
    }

    template<typename Character>
    inline std::shared_ptr<pattern::linear::Concatenation<Character>> shortcut<Character>::plc(const typename Traits<Character>::String &s) {
        std::list<typename pattern::Linear<Character>::Item> linear;
        for (auto i = s.cbegin(); i != s.cend(); ({
            linear.emplace_back(psc(*i));
            i++;
        }));
        return std::make_shared<plct>(std::move(linear));
    }

    template<typename Character>
    inline std::shared_ptr<pattern::KleeneClosure<Character>> shortcut<Character>::pk(const std::shared_ptr<regular::Pattern<Character>> &item) {
        return std::make_shared<pattern::KleeneClosure<Character>>(item);
    }

    template<typename Character>
    inline std::shared_ptr<pattern::Placeholder<Character>> shortcut<Character>::pp() {
        return std::make_shared<pattern::Placeholder<Character>>();
    }

    template<typename Character>
    inline std::shared_ptr<pattern::Collapsed<Character>> shortcut<Character>::pq(const std::shared_ptr<regular::Pattern<Character>> &p) {
        return std::make_shared<pattern::Collapsed<Character>>(p);
    }
}


