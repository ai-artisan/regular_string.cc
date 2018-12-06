#pragma once

#include <functional>
#include <list>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

#include "json/json.hh"

namespace rs {
    using Json=json::Json<>;

    class Regular : public std::enable_shared_from_this<Regular> {
        bool _merge = false;
    public:
        std::shared_ptr<Regular> merge() {
            return shared_from_this();
        }

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

            virtual Json json() {
                std::string s;
                s.resize(std::size_t(end - begin));
                for (auto i = begin; i != end; ({
                    *(s.begin() + (i - begin)) = *i;
                    i++;
                }));

                return s;
            }
        };

    protected:
        static std::stack<std::pair<
                std::shared_ptr<Regular>,
                std::function<void(std::shared_ptr<Match>)>
        >> _suffixes;
    public:
        std::shared_ptr<Match> match(
                const std::string::const_iterator &i,
                const std::string::const_iterator &i1
        ) {
            auto m = this->_match(i, i1);
            if (_merge) return std::make_shared<Match>(m->success, m->begin, m->end);
            else return m;
        }

    private:
        virtual std::shared_ptr<Match>
        _match(
                const std::string::const_iterator &,
                const std::string::const_iterator &
        ) = 0;
    };

    namespace regular {
        class Empty : public Regular {
            std::shared_ptr<Regular::Match>
            _match(
                    const std::string::const_iterator &i,
                    const std::string::const_iterator &
            ) final {
                return std::make_shared<Regular::Match>(true, i, i);
            }
        };

        class Singleton : public Regular {
            std::shared_ptr<Regular::Match>
            _match(
                    const std::string::const_iterator &i,
                    const std::string::const_iterator &i1
            ) final {
                return (i != i1 && _function(*i)) ? std::make_shared<Regular::Match>(true, i, std::next(i))
                                                  : std::make_shared<Regular::Match>(false, i, i);
            }

            const std::function<bool(const char &)> _function;
        public:
            explicit Singleton(std::function<bool(const char &)> function) : _function(std::move(function)) {}
        };

        class Linear : public Regular {
        protected:
            std::list<std::pair<
                    std::string,
                    std::shared_ptr<Regular>
            >> _list;
        public:
            std::shared_ptr<Linear> item(const std::shared_ptr<Regular> &item, std::string name = "") {
                _list.emplace_back(std::make_pair(name, item));
                return std::dynamic_pointer_cast<Linear>(shared_from_this());
            }
        };

        namespace linear {
            class Union : public Linear {
            public:
                class Match : public Regular::Match {
                public:
                    const std::string key;
                    const std::shared_ptr<Regular::Match> value;

                    Match(
                            const bool &success,
                            const std::string::const_iterator &begin,
                            const std::string::const_iterator &end,
                            std::string key,
                            std::shared_ptr<Regular::Match> value
                    ) :
                            Regular::Match(success, begin, end),
                            key(std::move(key)),
                            value(std::move(value)) {}
                };

            private:
                std::shared_ptr<Regular::Match>
                _match(
                        const std::string::const_iterator &i,
                        const std::string::const_iterator &i1
                ) final {
                    auto m = std::make_shared<Regular::Match>(false, i, i);
                    std::string key;
                    for (auto j = _list.cbegin(); j != _list.cend(); ({
                        m = j->second->match(i, i1);
                        if (m->success) {
                            key = j->first;
                            j = _list.cend();
                        } else j++;
                    }));
                    return std::make_shared<Match>(m->success, i, m->end, key, m);
                }
            };

            class Concatenation : public Linear {
            public:
                class Match : public Regular::Match {
                public:
                    const std::unordered_map<std::string, std::shared_ptr<Regular::Match>> map;

                    Match(
                            const bool &success,
                            const std::string::const_iterator &begin,
                            const std::string::const_iterator &end,
                            std::unordered_map<std::string, std::shared_ptr<Regular::Match>> &&map
                    ) :
                            Regular::Match(success, begin, end),
                            map(std::move(map)) {}
                };

            private:
                std::shared_ptr<Regular::Match>
                _match(
                        const std::string::const_iterator &i0,
                        const std::string::const_iterator &i1
                ) final {
                    std::unordered_map<std::string, std::shared_ptr<Regular::Match>> map;
                    auto i = i0;
                    std::shared_ptr<Regular::Match> m;
                    std::size_t k = 0;
                    for (auto j = _list.cbegin(); j != _list.cend(); ({
                        m = j->second->match(i, i1);
                        map[j->first] = m;
                        if (m->success) {
                            i = m->end;
                            k++;
                            j++;
                        } else j = _list.cend();
                    }));
                    return std::make_shared<Match>(m->success, i0, m->end, std::move(map));
                }
            };
        }

        class KleeneStar : public Regular {
        public:
            class Match : public Regular::Match {
            public:
                const std::list<std::shared_ptr<Regular::Match>> list;

                Match(
                        const bool &success,
                        const std::string::const_iterator &begin,
                        const std::string::const_iterator &end,
                        std::list<std::shared_ptr<Regular::Match>> &&list
                ) :
                        Regular::Match(success, begin, end),
                        list(std::move(list)) {}
            };

        private:
            std::shared_ptr<Regular::Match>
            _match(
                    const std::string::const_iterator &i0,
                    const std::string::const_iterator &i1
            ) final {
                std::list<std::shared_ptr<Regular::Match>> list;
                auto i = i0, end = i0;
                while (/*termination->match(i, i1) ? false :*/ ({
                    auto m = _repeat->match(i, i1);
                    m->success ? ({
                        list.emplace_back(m);
                        end = i = m->end;
                        true;
                    }) : false;
                }));
                return std::make_shared<Match>(true, i0, end, std::move(list));
            }

            const std::shared_ptr<Regular> _repeat;
        public:
            explicit KleeneStar(std::shared_ptr<Regular> repeat) : _repeat(std::move(repeat)) {}
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

    std::shared_ptr<regular::KleeneStar> RK(const std::shared_ptr<Regular> &repeat) {
        return std::make_shared<regular::KleeneStar>(repeat);
    }

    using RKM=typename regular::KleeneStar::Match;
}
