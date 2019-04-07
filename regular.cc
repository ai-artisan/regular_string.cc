#include "regular.h"

namespace regular {
    template<typename Character>
    template<typename Derived>
    inline std::shared_ptr<Derived> Record<Character>::as() const {
        return std::static_pointer_cast<Derived>(this->shared_from_this());
    }

    namespace pattern {
        template<typename Character>
        std::pair<bool, std::shared_ptr<Record < Character>>>

        inline Empty<Character>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            return std::pair{true, std::make_shared<Record<Character>>(begin)};
        }

        template<typename Character>
        std::pair<bool, std::shared_ptr<Record < Character>>>

        Singleton<Character>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            return (begin != end && describe(*begin))
                   ? std::pair{true, std::make_shared<Record<Character>>(std::next(begin))}
                   : std::pair{false, std::make_shared<Record<Character>>(begin)};
        }

        namespace singleton {
            template<typename Character, typename Context>
            Closure<Character, Context>::Closure(Context &&context, const decltype(depict) &depict):
                    Singleton<Character>([&](const Character &c) { return this->depict(this->context, c); }), context(std::move(context)), depict(depict) {}
        }

        namespace binary {
            template<typename Character>
            std::pair<bool, std::shared_ptr<Record < Character>>>

            Union<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                auto pair = this->binary[0]->match(begin, end);
                if (pair.first) return std::pair{true, std::make_shared<record::SetBinary<Character>>(pair.second->end, 0, pair.second)};
                else {
                    pair = this->binary[1]->match(begin, end);
                    return std::pair{pair.first, std::make_shared<record::SetBinary<Character>>(pair.second->end, 1, pair.second)};
                }
            }

            template<typename Character>
            std::pair<bool, std::shared_ptr<Record < Character>>>

            Concatenation<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                std::array<std::shared_ptr<Record<Character>>, 2> array = {nullptr, nullptr};
                auto pair = this->binary[0]->match(begin, end);
                array[0] = pair.second;
                if (pair.first) {
                    pair = this->binary[1]->match(pair.second->end, end);
                    array[1] = pair.second;
                }
                return std::pair{pair.first, std::make_shared<record::ConcatenationBinary<Character>>(array[1]->end, std::move(array))};
            }


            template<typename Character>
            std::pair<bool, std::shared_ptr<Record < Character>>>

            Intersection<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                bool index = false;
                auto pair = this->binary[0]->match(begin, end);
                if (pair.first) {
                    index = true;
                    pair = this->binary[1]->match(begin, pair.second->end);
                }
                return std::pair{pair.first, std::make_shared<record::SetBinary<Character>>(pair.second->end, index, pair.second)};
            }

            template<typename Character>
            std::pair<bool, std::shared_ptr<Record < Character>>>

            Difference<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                bool index = false;
                auto pair = this->binary[0]->match(begin, end);
                if (pair.first) {
                    index = true;
                    auto pair1 = this->binary[1]->match(begin, pair.second->end);
                    if (pair1.first) pair = {false, pair1.second};
                }
                return std::pair{pair.first, std::make_shared<record::SetBinary<Character>>(pair.second->end, index, pair.second)};
            }
        }

        template<typename Character>
        std::pair<bool, std::shared_ptr<Record < Character>>>

        KleeneClosure<Character>::match(
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
            return std::pair{true, std::make_shared<record::KleeneClosure<Character>>(j, std::move(list))};
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

        inline std::shared_ptr<pattern::singleton::Closure<char, char>> psi(const char &c0) {
            return std::make_shared<pattern::singleton::Closure<char, char>>(char(c0), [](const char &c0, const char &c) -> bool {
                return c == c0;
            });
        }

        std::shared_ptr<pattern::singleton::Closure<char, Traits<char>::String>> psi(Traits<char>::String &&s) {
            return std::make_shared<pattern::singleton::Closure<char, Traits<char>::String>>(std::move(s), [&](const Traits<char>::String &s, const char &c) -> bool {
                for (auto i = s.cbegin(); i != s.cend(); ({
                    if (c == *i) return true;
                    i++;
                }));
                return false;
            });
        }

        std::shared_ptr<pattern::singleton::Closure<char, std::array<char, 2>>> psr(const char &inf, const char &sup) {
            return std::make_shared<pattern::singleton::Closure<char, std::array<char, 2>>>(std::array{inf, sup}, [&](const std::array<char, 2> &interval, const char &c) -> bool {
                return interval[0] <= c && c <= interval[1];
            });
        }

        std::shared_ptr<pattern::singleton::Closure<
                char,
                std::list<std::shared_ptr<pattern::Singleton<char>>>
        >> psu(std::list<std::shared_ptr<pattern::Singleton<char>>> &&list) {
            return std::make_shared<pattern::singleton::Closure<
                    char,
                    std::list<std::shared_ptr<pattern::Singleton<char>>>
            >>(std::move(list), [&](const std::list<std::shared_ptr<pattern::Singleton<char>>> &list, const char &c) -> bool {
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
        >> psi(std::list<std::shared_ptr<pattern::Singleton<char>>> &&list) {
            return std::make_shared<pattern::singleton::Closure<
                    char,
                    std::list<std::shared_ptr<pattern::Singleton<char>>>
            >>(std::move(list), [&](const std::list<std::shared_ptr<pattern::Singleton<char>>> &list, const char &c) -> bool {
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
        >> psd(std::list<std::shared_ptr<pattern::Singleton<char>>> &&list) {
            return std::make_shared<pattern::singleton::Closure<
                    char,
                    std::list<std::shared_ptr<pattern::Singleton<char>>>
            >>(std::move(list), [&](const std::list<std::shared_ptr<pattern::Singleton<char>>> &list, const char &c) -> bool {
                bool b = false;
                for (auto i = list.cbegin(); i != list.cend(); ({
                    std::cout << (*i)->describe(c) << ' ' << b << "-----\n";
                    if (!((*i)->describe(c) xor b)) return false;
                    b = !b;
                    i++;
                }));
                return true;
            });
        }
    }
}


