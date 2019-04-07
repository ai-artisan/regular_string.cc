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

        template<typename Character, typename Context>
        std::pair<bool, std::shared_ptr<Record < Character>>>

        Singleton<Character, Context>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            return (begin != end && describe(*begin, context))
                   ? std::pair{true, std::make_shared<Record<Character>>(std::next(begin))}
                   : std::pair{false, std::make_shared<Record<Character>>(begin)};
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

            Complement<Character>::match(
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

//        template<typename Context>
//        inline std::shared_ptr<pattern::Singleton<char, Context>> ps(const std::function<bool(const char &, const Context &)> &callback, Context &&context) {
//            return std::make_shared<pattern::Singleton<char, Context>>(callback, std::forward<Context>(context));
//        }
//
//        inline std::shared_ptr<pattern::Singleton<char, nullptr_t>> psa() {
//            return std::make_shared<pattern::Singleton<char, nullptr_t>>([](const char &, const nullptr_t &) {
//                return true;
//            }, nullptr);
//        }
//
//        std::shared_ptr<pattern::Singleton<char, std::pair<char, bool>>> psi(const char &c, const bool &is) {
//            return std::make_shared<pattern::Singleton<char, std::pair<char, bool>>>(
//                    std::pair{c, is},
//                    [&](const char &c, const std::pair<char, bool> &si) -> bool {
//                        return (!si.second) xor (c == si.first);
//                    }
//            );
//        }
//
//        std::shared_ptr<pattern::Singleton<char, std::pair<Traits<char>::String, bool>>> psi(const Traits<char>::String &s, const bool &in) {
//            return std::make_shared<pattern::Singleton<char, std::pair<Traits<char>::String, bool>>>(
//                    std::pair{s, in},
//                    [&](const char &c, const std::pair<Traits<char>::String, bool> &si) -> bool {
//                        for (auto i = si.first.cbegin(); i != si.first.cend(); ({
//                            if (c == *i) return si.second;
//                            i++;
//                        }));
//                        return !si.second;
//                    }
//            );
//        }


    }
}


