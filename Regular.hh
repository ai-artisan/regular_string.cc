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
            std::shared_ptr<Derived> as() {
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
                std::function<void(const std::shared_ptr<Match> &)>
        >> _suffixes;
    public:
        template<typename Derived>
        std::shared_ptr<Derived> as() {
            return std::static_pointer_cast<Derived>(shared_from_this());
        }

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

        template<typename Context= nullptr_t>
        class Singleton : public Regular {
            std::shared_ptr<Regular::Match>
            _match(
                    const std::string::const_iterator &i,
                    const std::string::const_iterator &i1
            ) final {
                return (i != i1 && _function(_context, *i))
                       ? std::make_shared<Regular::Match>(true, i, i + 1)
                       : std::make_shared<Regular::Match>(false, i, i);
            }

            const Context _context;
            const std::function<bool(const Context &, const char &)> _function;
        public:
            Singleton(
                    Context context,
                    std::function<bool(const Context &, const char &)> function
            ) : _context(std::move(context)),
                _function(std::move(function)) {}
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
                    bool success;
                    std::string::const_iterator end;
                    std::unordered_map<std::string, std::shared_ptr<Regular::Match>> map;

                    std::function<void(const std::string::const_iterator &)> lambda = [&](
                            const std::string::const_iterator &i
                    ) -> void {
                        if (_list.empty()) {
                            success = true;
                            end = i;
                        } else {
                            auto front = _list.front();
                            _list.pop_front();
                            std::shared_ptr<Match> suffix_match = nullptr;
                            _suffixes.emplace(std::make_pair(
                                    shared_from_this(),
                                    [&](const std::shared_ptr<Regular::Match> &m) -> void {
                                        suffix_match = m->as<Match>();
                                    }
                            ));
                            auto prefix_match = front.second->match(i, i1);
                            map[front.first] = prefix_match;
                            if ((success = prefix_match->success)) {
                                if (suffix_match) {
                                    success = suffix_match->success;
                                    end = suffix_match->end;
                                    auto &suffix_map = (std::unordered_map<
                                            std::string,
                                            std::shared_ptr<Regular::Match>
                                    > &) suffix_match->map;
                                    for (auto j = map.cbegin(); j != map.cend(); ({
                                        suffix_map[j->first] = j->second;
                                        j++;
                                    }));
                                    map = std::move(suffix_map);
                                } else lambda(prefix_match->end);
                            } else end = prefix_match->end;

                            _suffixes.pop();
                            _list.emplace_front(front);
                        }
                    };

                    lambda(i0);
                    return std::make_shared<Match>(success, i0, end, std::move(map));
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
                while (({
                    auto m = _suffixes.top().first->match(i, i1);
                    _suffixes.top().second(m);
                    m->success ? false : ({
                        m = _repeat->match(i, i1);
                        m->success ? ({
                            list.emplace_back(m);
                            end = i = m->end;
                            true;
                        }) : false;
                    });
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

    std::shared_ptr<regular::Singleton<bool>> RS(const bool &any = true) {
        return std::make_shared<regular::Singleton<bool>>(
                any,
                [&](const bool &any, const char &t) -> bool { return any; }
        );
    }

    namespace context {
        struct IsCharacter {
            const bool is;
            const char character;
        };
    }

    std::shared_ptr<regular::Singleton<context::IsCharacter>> RS(const char &c, const bool &is = true) {
        using namespace context;
        return std::make_shared<regular::Singleton<IsCharacter>>(
                IsCharacter{is, c},
                [&](const IsCharacter &c, const char &t) -> bool {
                    return (!c.is) xor (t == c.character);
                }
        );
    }

    namespace context {
        struct InString {
            const bool in;
            const std::string string;
        };
    }

    std::shared_ptr<regular::Singleton<context::InString>> RS(const std::string &s, const bool &in = true) {
        using namespace context;
        return std::make_shared<regular::Singleton<context::InString>>(
                InString{in, s},
                [&](const InString &c, const char &t) -> bool {
                    for (auto i = c.string.cbegin(); i != c.string.cend(); ({
                        if (t == *i) return c.in;
                        i++;
                    }));
                    return !c.in;
                }
        );
    }

    namespace context {
        struct Interval {
            const char inf;
            const char sup;
        };
    }

    std::shared_ptr<regular::Singleton<context::Interval>> RS(const char &inf, const char &sup) {
        using namespace context;
        return std::make_shared<regular::Singleton<Interval>>(
                Interval{inf, sup},
                [&](const Interval &c, const char &t) -> bool { return t >= c.inf && t < c.sup; }
        );
    }

    namespace context {
        using IntervalList=std::list<std::pair<char, char>>;
    }

    std::shared_ptr<regular::Singleton<context::IntervalList>> RS(const std::list<std::pair<char, char>> &list) {
        using namespace context;
        return std::make_shared<regular::Singleton<IntervalList >>(
                list,
                [&](const IntervalList &c, const char &t) -> bool {
                    for (auto i = c.cbegin(); i != c.cend(); ({
                        if (t >= i->first && t < i->second) return true;
                        i++;
                    }));
                    return false;
                }
        );
    }

    template<typename Context>
    using RSM=typename regular::Singleton<Context>::Match;

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

    decltype(Regular::_suffixes) Regular::_suffixes = []() {
        auto self = decltype(Regular::_suffixes)();
        self.emplace(std::make_pair(RS(false), [](const std::shared_ptr<Match> &) -> void {}));
        return self;
    }();
}
