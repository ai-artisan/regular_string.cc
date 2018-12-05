#pragma once

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace rs {
    class Regular {
    public:
        class Match : public std::enable_shared_from_this<Match> {
        public:
            const bool success;

            const std::string::const_iterator begin, end;

            explicit Match(
                    const bool &success,
                    const std::string::const_iterator &begin,
                    const std::string::const_iterator &end
            ) :
                    success(success),
                    begin(begin),
                    end(end) {}

            template<typename Derived>
            std::shared_ptr<Derived> derived() {
                return std::static_pointer_cast<Derived>(shared_from_this());
            }
        };

        virtual std::shared_ptr<Match>
        match(const std::string::const_iterator &, const std::string::const_iterator &) = 0;
    };

    namespace regular {
        class Empty : public Regular {
        public:
            std::shared_ptr<Regular::Match>
            match(const std::string::const_iterator &i, const std::string::const_iterator &) final {
                return std::make_shared<Regular::Match>(true, i, i);
            }
        };

        class Singleton : public Regular {
            std::function<bool(const char &)> _function;
        public:
            explicit Singleton(decltype(_function) function) {
                _function = std::move(function);
            }

            std::shared_ptr<Regular::Match>
            match(const std::string::const_iterator &i, const std::string::const_iterator &end) final {
                return (i != end && _function(*i)) ? std::make_shared<Regular::Match>(true, i, std::next(i))
                                                   : std::make_shared<Regular::Match>(false, i, i);
            }
        };

        class Linear : public Regular, public std::enable_shared_from_this<Linear> {
        protected:
            std::list<std::shared_ptr<Regular>> _list;
            std::unordered_map<std::size_t, std::string> _map;
        public:
            std::shared_ptr<Linear> item(const std::shared_ptr<Regular> &item, std::string name = "") {
                _list.emplace_back(item);
                _map[_list.size() - 1] = std::move(name);
                return shared_from_this();
            }
        };

        namespace linear {
            class Union : public Linear {
            public:
                class Match : public Regular::Match {
                public:
                    const std::size_t index;
                    const std::string key;

                    Match(
                            const bool &success,
                            const std::string::const_iterator &begin,
                            const std::string::const_iterator &end,
                            const std::size_t &index,
                            std::string key
                    ) :
                            Regular::Match(success, begin, end),
                            index(index),
                            key(std::move(key)) {}
                };

                std::shared_ptr<Regular::Match>
                match(const std::string::const_iterator &i, const std::string::const_iterator &end) final {
                    auto m = std::make_shared<Regular::Match>(false, i, i);
                    std::size_t k = 0;
                    for (auto j = _list.cbegin(); j != _list.cend(); ({
                        m = (*j)->match(i, end);
                        if (m->success) j = _list.cend();
                        else {
                            k++;
                            j++;
                        }
                    }));
                    return std::make_shared<Match>(m->success, i, m->end, k, _map[k]);
                }
            };

            class Concatenation : public Linear {
            public:
                class Match : public Regular::Match {
                public:
                    const std::vector<std::shared_ptr<Regular::Match>> vector;
                    const std::unordered_map<std::string, std::shared_ptr<Regular::Match>> map;

                    Match(
                            const bool &success,
                            const std::string::const_iterator &begin,
                            const std::string::const_iterator &end,
                            std::vector<std::shared_ptr<Regular::Match>> &&vector,
                            std::unordered_map<std::string, std::shared_ptr<Regular::Match>> &&map
                    ) :
                            Regular::Match(success, begin, end),
                            vector(std::move(vector)),
                            map(std::move(map)) {}
                };

                std::shared_ptr<Regular::Match>
                match(const std::string::const_iterator &i0, const std::string::const_iterator &end) final {
                    std::vector<std::shared_ptr<Regular::Match>> vector(_list.size());
                    std::unordered_map<std::string, std::shared_ptr<Regular::Match>> map;
                    auto i = i0;
                    std::shared_ptr<Regular::Match> m;
                    std::size_t k = 0;
                    for (auto j = _list.cbegin(); j != _list.cend(); ({
                        m = (*j)->match(i, end);
                        vector[k] = m;
                        map[_map[k]] = m;
                        if (m->success) {
                            i = m->end;
                            k++;
                            j++;
                        } else j = _list.cend();
                    }));
                    return std::make_shared<Match>(m->success, i0, m->end, std::move(vector), std::move(map));
                }
            };
        }

        class KleeneStar : public Regular {
            std::shared_ptr<Regular> _repeat;
            std::shared_ptr<Regular> _termination;
        public:
            explicit KleeneStar(
                    const std::shared_ptr<Regular> &repeat,
                    const std::shared_ptr<Regular> &termination
            ) {
                _repeat = repeat;
                _termination = termination;
            }

            class Match : public Regular::Match {
            public:
                const std::list<std::shared_ptr<Regular::Match>> repeats;
                const std::shared_ptr<Regular::Match> termination;

                Match(
                        const bool &success,
                        const std::string::const_iterator &begin,
                        const std::string::const_iterator &end,
                        std::list<std::shared_ptr<Regular::Match>> &&repeats,
                        std::shared_ptr<Regular::Match> termination
                ) :
                        Regular::Match(success, begin, end),
                        repeats(std::move(repeats)),
                        termination(std::move(termination)) {}
            };

            std::shared_ptr<Regular::Match>
            match(const std::string::const_iterator &i0, const std::string::const_iterator &end) final {
                std::list<std::shared_ptr<Regular::Match>> repeats;
                std::shared_ptr<Regular::Match> m = std::make_shared<Regular::Match>(false, i0, i0), termination = m;
                auto i = i0;
                while (({
                    termination = m = _termination->match(i, end);
                    m->success ? false : ({
                        m = _repeat->match(i, end);
                        m->success ? ({
                            repeats.emplace_back(m);
                            i = m->end;
                            true;
                        }) : false;
                    });
                }));
                return std::make_shared<Match>(true, i0, m->end, std::move(repeats), termination);
            }
        };
    }

    using RM=typename Regular::Match;

    std::shared_ptr<regular::Empty> RE() {
        return std::make_shared<regular::Empty>();
    }

    using REM=typename regular::Empty::Match;

    std::shared_ptr<regular::Singleton> RS(const bool &any = true) {
        return std::make_shared<regular::Singleton>([&](const char &t) -> bool { return any; });
    }

    std::shared_ptr<regular::Singleton> RS(const char &c, const bool &is = true) {
        return std::make_shared<regular::Singleton>([&](const char &t) -> bool { return is xor t == c; });
    }

    std::shared_ptr<regular::Singleton> RS(const std::string &s, const bool &in = true) {
        return std::make_shared<regular::Singleton>([&](const char &t) -> bool {
            for (auto i = s.cbegin(); i != s.cend(); ({
                if (t == *i) return in;
                i++;
            }));
            return !in;
        });
    }

    std::shared_ptr<regular::Singleton> RS(const char &inf, const char &sup) {
        return std::make_shared<regular::Singleton>([&](const char &t) -> bool { return t >= inf && t < sup; });
    }

    std::shared_ptr<regular::Singleton> RS(const std::list<std::pair<char, char>> &list) {
        return std::make_shared<regular::Singleton>([&](const char &t) -> bool {
            for (auto i = list.cbegin(); i != list.cend(); ({
                if (t >= i->first && t < i->second) return true;
                i++;
            }));
            return false;
        });
    }

    using RSM=typename regular::Singleton::Match;

    std::shared_ptr<regular::linear::Union> RU() {
        return std::make_shared<regular::linear::Union>();
    }

    using RUM=typename regular::linear::Union::Match;

    std::shared_ptr<regular::linear::Concatenation> RC() {
        return std::make_shared<regular::linear::Concatenation>();
    }

    std::shared_ptr<regular::linear::Concatenation> RC(const std::string &raw) {
        auto rc = RC();

        for (auto i = raw.cbegin(); i != raw.cend(); ({
            rc->item(RS(*i));
            i++;
        }));
        return rc;
    }

    using RCM=typename regular::linear::Concatenation::Match;

    std::shared_ptr<regular::KleeneStar> RK(
            const std::shared_ptr<Regular> &repeat,
            const std::shared_ptr<Regular> &termination = RS(false)
    ) {
        return std::make_shared<regular::KleeneStar>(repeat, termination);
    }

    using RKM=typename regular::KleeneStar::Match;
}
