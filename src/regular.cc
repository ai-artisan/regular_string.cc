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
            return {true, std::make_shared<Record<Character>>(begin, begin)};
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
            return {success, std::make_shared<Record<Character>>(begin, end1)};
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
                return {success, std::make_shared<record::LinearSome<Character>>(begin, end1, std::move(key), record)};
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
                return {success, std::make_shared<record::LinearEvery<Character>>(begin, end1, std::move(map), std::move(list))};
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

                return {!(success xor sign), std::make_shared<record::LinearSome<Character>>(begin, end1, std::move(key), record)};
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
                return {success, std::make_shared<record::LinearEvery<Character>>(begin, end1, std::move(map), std::move(list))};
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
            return {true, std::make_shared<record::KleeneClosure<Character>>(begin, end1, std::move(list))};
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
            return {matched.success, std::make_shared<Record<Character>>(begin, matched.record->end)};
        }
    }


    template<typename Character>
    inline std::shared_ptr<typename hub<Character>::pot> hub<Character>::po() {
        return std::make_shared<pot>();
    }

    template<typename Character>
    inline std::shared_ptr<typename hub<Character>::pst> hub<Character>::ps(const std::function<bool(const Character &)> &describe) {
        return std::make_shared<pst>(describe);
    }

    template<typename Character>
    template<typename Context>
    inline std::shared_ptr<pattern::singleton::Closure<Character, Context>> hub<Character>::ps(Context &&context, const std::function<bool(const Context &, const Character &)> &depict) {
        return std::make_shared<hub<Character>::psct<Context>>(std::forward<Context>(context), depict);
    }

    template<typename Character>
    inline std::shared_ptr<typename hub<Character>::pst> hub<Character>::psa() {
        return std::make_shared<pst>([](const Character &) {
            return true;
        });
    }

    template<typename Character>
    inline std::shared_ptr<pattern::singleton::Closure<Character, Character>> hub<Character>::psc(const Character &c0) {
        return std::make_shared<hub<Character>::psct<Character>>(Character(c0), [](const Character &c0, const Character &c) -> bool {
            return c == c0;
        });
    }

    template<typename Character>
    std::shared_ptr<pattern::singleton::Closure<Character, typename Traits<Character>::String>> hub<Character>::pss(typename Traits<Character>::String &&s) {
        return std::make_shared<hub<Character>::psct<typename Traits<Character>::String>>(std::move(s), [&](const typename Traits<Character>::String &s, const Character &c) -> bool {
            for (auto i = s.cbegin(); i != s.cend(); ({
                if (c == *i) return true;
                i++;
            }));
            return false;
        });
    }

    template<typename Character>
    inline std::shared_ptr<pattern::singleton::Closure<Character, std::array<Character, 2>>> hub<Character>::psr(const Character &inf, const Character &sup) {
        return std::make_shared<hub<Character>::psct<std::array<Character, 2>>>(std::array{inf, sup}, [&](const std::array<Character, 2> &interval, const Character &c) -> bool {
            return interval[0] <= c && c <= interval[1];
        });
    }

    template<typename Character>
    std::shared_ptr<pattern::singleton::Closure<Character,
            std::list<std::shared_ptr<typename hub<Character>::pst>>
    >> hub<Character>::psu(std::list<std::shared_ptr<pst>> &&list) {
        return std::make_shared<pattern::singleton::Closure<
                Character,
                std::list<std::shared_ptr<pst>>
        >>(std::move(list), [&](const std::list<std::shared_ptr<pst>> &list, const Character &c) -> bool {
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
            std::list<std::shared_ptr<typename hub<Character>::pst>>
    >> hub<Character>::psi(std::list<std::shared_ptr<typename hub<Character>::pst>> &&list) {
        return std::make_shared<pattern::singleton::Closure<
                Character,
                std::list<std::shared_ptr<pst>>
        >>(std::move(list), [&](const std::list<std::shared_ptr<pst>> &list, const Character &c) -> bool {
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
            std::pair<
                    std::list<std::shared_ptr<typename hub<Character>::pst>>,
                    bool
            >
    >> hub<Character>::psd(std::list<std::shared_ptr<typename hub<Character>::pst>> &&list, const bool &sign) {
        return std::make_shared<hub::psct<std::pair<
                std::list<std::shared_ptr<hub::pst>>,
                bool
        >>>(std::pair{std::move(list), sign}, [&](const std::pair<
                std::list<std::shared_ptr<hub::pst>>,
                bool
        > &pair, const Character &c) -> bool {
            auto&[l, s]=pair;
            bool b = false;
            for (auto i = l.crbegin(); i != l.crend(); ({
                b = (*i)->describe(c) && !b;
                i++;
            }));
            return !(b xor s);
        });
    }

    template<typename Character>
    inline std::shared_ptr<typename hub<Character>::plut> hub<Character>::plu(std::list<typename plt::Item> &&list) {
        return std::make_shared<plut>(std::move(list));
    }

    template<typename Character>
    inline std::shared_ptr<typename hub<Character>::plit> hub<Character>::pli(std::list<typename plt::Item> &&list) {
        return std::make_shared<plit>(std::move(list));
    }

    template<typename Character>
    inline std::shared_ptr<typename hub<Character>::pldt> hub<Character>::pld(std::list<typename plt::Item> &&list, const bool &sign) {
        return std::make_shared<pldt>(std::move(list), sign);
    }

    template<typename Character>
    inline std::shared_ptr<typename hub<Character>::plct> hub<Character>::plc(std::list<typename plt::Item> &&list) {
        return std::make_shared<plct>(std::move(list));
    }

    template<typename Character>
    inline std::shared_ptr<typename hub<Character>::plct> hub<Character>::plc(const typename Traits<Character>::String &s) {
        std::list<typename plt::Item> linear;
        for (auto i = s.cbegin(); i != s.cend(); ({
            linear.emplace_back(psc(*i));
            i++;
        }));
        return std::make_shared<plct>(std::move(linear));
    }

    template<typename Character>
    inline std::shared_ptr<typename hub<Character>::pkt> hub<Character>::pk(const std::shared_ptr<pt> &item) {
        return std::make_shared<pkt>(item);
    }

    template<typename Character>
    inline std::shared_ptr<typename hub<Character>::ppt> hub<Character>::pp() {
        return std::make_shared<ppt>();
    }

    template<typename Character>
    inline std::shared_ptr<typename hub<Character>::pqt> hub<Character>::pq(const std::shared_ptr<pt> &p) {
        return std::make_shared<pqt>(p);
    }
}

#undef TYPE
