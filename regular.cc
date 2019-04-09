#include "regular.h"

namespace regular {
    template<typename Character>
    template<typename Derived>
    inline std::shared_ptr<Derived> Record<Character>::as() const {
        return std::dynamic_pointer_cast<Derived>(const_cast<Record<Character> *>(this)->shared_from_this());
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
                typename Traits<Character>::String key = "";
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
    }

    namespace shortcut {
        inline std::shared_ptr<pattern::Empty<char>> pe() {
            return std::make_shared<pattern::Empty<char>>();
        }

        inline std::shared_ptr<pattern::Singleton<char>> ps(const std::function<bool(const char &)> &describe) {
            return std::make_shared<pattern::Singleton<char>>(describe);
        }

        template<typename Context>
        inline std::shared_ptr<pattern::singleton::Closure<char, Context>> ps(Context &&context, const decltype(pattern::singleton::Closure<char, Context>::depict) &depict) {
            return std::make_shared<pattern::singleton::Closure<char, Context>>(std::forward<Context>(context), depict);
        }

        inline std::shared_ptr<pattern::Singleton<char>> psa() {
            return std::make_shared<pattern::Singleton<char>>([](const char &) {
                return true;
            });
        }

        inline std::shared_ptr<pattern::singleton::Closure<char, char>> psc(const char &c0) {
            return std::make_shared<pattern::singleton::Closure<char, char>>(char(c0), [](const char &c0, const char &c) -> bool {
                return c == c0;
            });
        }

        std::shared_ptr<pattern::singleton::Closure<char, Traits<char>::String>> pss(Traits<char>::String &&s) {
            return std::make_shared<pattern::singleton::Closure<char, Traits<char>::String>>(std::move(s), [&](const Traits<char>::String &s, const char &c) -> bool {
                for (auto i = s.cbegin(); i != s.cend(); ({
                    if (c == *i) return true;
                    i++;
                }));
                return false;
            });
        }

        inline std::shared_ptr<pattern::singleton::Closure<char, std::array<char, 2>>> psr(const char &inf, const char &sup) {
            return std::make_shared<pattern::singleton::Closure<char, std::array<char, 2>>>(std::array{inf, sup}, [&](const std::array<char, 2> &interval, const char &c) -> bool {
                return interval[0] <= c && c <= interval[1];
            });
        }

        std::shared_ptr<pattern::singleton::Closure<
                char,
                std::list<std::shared_ptr<pattern::Singleton<char>>>
        >> psu(std::initializer_list<std::shared_ptr<pattern::Singleton<char>>> list) {
            return std::make_shared<pattern::singleton::Closure<
                    char,
                    std::list<std::shared_ptr<pattern::Singleton<char>>>
            >>(list, [&](const std::list<std::shared_ptr<pattern::Singleton<char>>> &list, const char &c) -> bool {
                for (auto i = list.cbegin(); i != list.cend(); ({
                    if ((*i)->describe(c)) return true;
                    i++;
                }));
                return false;
            });
        }

        std::shared_ptr<pattern::singleton::Closure<
                char,
                std::list<std::shared_ptr<pattern::Singleton<char>>>
        >> psi(std::initializer_list<std::shared_ptr<pattern::Singleton<char>>> list) {
            return std::make_shared<pattern::singleton::Closure<
                    char,
                    std::list<std::shared_ptr<pattern::Singleton<char>>>
            >>(list, [&](const std::list<std::shared_ptr<pattern::Singleton<char>>> &list, const char &c) -> bool {
                for (auto i = list.cbegin(); i != list.cend(); ({
                    if (!(*i)->describe(c)) return false;
                    i++;
                }));
                return true;
            });
        }

        std::shared_ptr<pattern::singleton::Closure<
                char,
                std::list<std::shared_ptr<pattern::Singleton<char>>>
        >> psd(std::initializer_list<std::shared_ptr<pattern::Singleton<char>>> list) {
            return std::make_shared<pattern::singleton::Closure<
                    char,
                    std::list<std::shared_ptr<pattern::Singleton<char>>>
            >>(list, [&](const std::list<std::shared_ptr<pattern::Singleton<char>>> &list, const char &c) -> bool {
                bool b = false;
                for (auto i = list.crbegin(); i != list.crend(); ({
                    b = (*i)->describe(c) && !b;
                    i++;
                }));
                return b;
            });
        }

        inline std::shared_ptr<pattern::binary::Union<char>> pbu(const std::shared_ptr<regular::Pattern<char>> &p0, const std::shared_ptr<regular::Pattern<char>> &p1) {
            return std::make_shared<pattern::binary::Union<char>>(std::array{p0, p1});
        }

        inline std::shared_ptr<pattern::binary::Intersection<char>> pbi(const std::shared_ptr<regular::Pattern<char>> &p0, const std::shared_ptr<regular::Pattern<char>> &p1) {
            return std::make_shared<pattern::binary::Intersection<char>>(std::array{p0, p1});
        }

        inline std::shared_ptr<pattern::binary::Difference<char>> pbd(const std::shared_ptr<regular::Pattern<char>> &p0, const std::shared_ptr<regular::Pattern<char>> &p1) {
            return std::make_shared<pattern::binary::Difference<char>>(std::array{p0, p1});
        }

        inline std::shared_ptr<pattern::binary::Concatenation<char>> pbc(const std::shared_ptr<regular::Pattern<char>> &p0, const std::shared_ptr<regular::Pattern<char>> &p1) {
            return std::make_shared<pattern::binary::Concatenation<char>>(std::array{p0, p1});
        }

        inline std::shared_ptr<pattern::linear::Union<char>> plu(std::initializer_list<pattern::Linear<char>::Item> list) {
            return std::make_shared<pattern::linear::Union<char>>(list);
        }

        inline std::shared_ptr<pattern::linear::Intersection<char>> pli(std::initializer_list<pattern::Linear<char>::Item> list) {
            return std::make_shared<pattern::linear::Intersection<char>>(list);
        }

        inline std::shared_ptr<pattern::linear::Difference<char>> pld(std::initializer_list<pattern::Linear<char>::Item> list) {
            return std::make_shared<pattern::linear::Difference<char>>(list);
        }

        inline std::shared_ptr<pattern::linear::Concatenation<char>> plc(std::initializer_list<pattern::Linear<char>::Item> list) {
            return std::make_shared<pattern::linear::Concatenation<char>>(list);
        }

        inline std::shared_ptr<pattern::KleeneClosure<char>> pk(const std::shared_ptr<regular::Pattern<char>> &item) {
            return std::make_shared<pattern::KleeneClosure<char>>(item);
        }
    }
}


