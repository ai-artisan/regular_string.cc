#pragma once

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace regular_string {
    class Regular : public std::enable_shared_from_this<Regular> {
        std::string _label;
    public:
        std::shared_ptr<Regular> label(std::string label) {
            _label = std::move(label);
            return shared_from_this();
        }

        const std::string &label() const { return _label; }

        class Match {
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

        class Linear : public Regular {
        protected:
            std::vector<std::shared_ptr<Regular>> _vector;
            std::unordered_map<std::size_t, std::string> _map;
        public:
            explicit Linear(std::vector<std::shared_ptr<Regular>> vector) {
                for (auto i = 0; i < vector.size(); ({
                    _map[i] = vector[i]->label();
                    i++;
                }));
                _vector = std::move(vector);
            }
        };

        namespace linear {
            class Union : public Linear {
            public:
                explicit Union(std::vector<std::shared_ptr<Regular>> vector) : Linear(std::move(vector)) {}

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
                    std::size_t j = 0;
                    for (; j < _vector.size(); ({
                        m = _vector[j]->match(i, end);
                        if (m->success) j = _vector.size();
                        else j++;
                    }));
                    return std::make_shared<Match>(m->success, i, m->end, j, _map[j]);
                }
            };

            class Concatenation : public Linear {
            public:
                explicit Concatenation(std::vector<std::shared_ptr<Regular>> vector) : Linear(std::move(vector)) {}

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
                    std::vector<std::shared_ptr<Regular::Match>> vector(_vector.size());
                    std::unordered_map<std::string, std::shared_ptr<Regular::Match>> map;
                    auto i = i0;
                    std::shared_ptr<Regular::Match> m;
                    for (std::size_t j = 0; j < _vector.size(); ({
                        m = _vector[j]->match(i, end);
                        vector[j] = m;
                        map[_map[j]] = m;
                        if (m->success) {
                            i = m->end;
                            j++;
                        } else j = _vector.size();
                    }));
                    return std::make_shared<Match>(m->success, i0, m->end, std::move(vector), std::move(map));
                }
            };
        }

        class KleeneStar : public Regular {
            std::shared_ptr<Regular> _regular;
        public:
            explicit KleeneStar(const std::shared_ptr<Regular> &regular) {
                _regular = regular;
            }

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

            std::shared_ptr<Regular::Match>
            match(const std::string::const_iterator &i0, const std::string::const_iterator &end) final {
                std::list<std::shared_ptr<Regular::Match>> list;
                auto i = i0;
                std::shared_ptr<Regular::Match> m;
                while (({
                    m = _regular->match(i, end);
                    m->success ? ({
                        list.emplace_back(m);
                        i = m->end;
                        true;
                    }) : false;
                }));
                return std::make_shared<Match>(true, i0, m->end, std::move(list));
            }
        };
    }

    std::shared_ptr<Regular> re() {
        return std::make_shared<regular::Empty>();
    }

    std::shared_ptr<Regular> rs(const char &c) {
        return std::make_shared<regular::Singleton>([&](const char &t) -> bool { return t == c; });
    }

    std::shared_ptr<Regular> rs(const char &inf, const char &sup) {
        return std::make_shared<regular::Singleton>([&](const char &t) -> bool { return t >= inf && t < sup; });
    }

    std::shared_ptr<Regular> rs(const std::list<std::pair<char, char>> &list) {
        return std::make_shared<regular::Singleton>([&](const char &t) -> bool {
            for (auto i = list.cbegin(); i != list.cend(); ({
                if (t >= i->first && t < i->second) return true;
                i++;
            }));
            return false;
        });
    }

    std::shared_ptr<Regular> ru(std::vector<std::shared_ptr<Regular>> vector) {
        return std::make_shared<regular::linear::Union>(std::move(vector));
    }

    std::shared_ptr<Regular> rc(std::vector<std::shared_ptr<Regular>> vector) {
        return std::make_shared<regular::linear::Concatenation>(std::move(vector));
    }

    std::shared_ptr<Regular> rc(const std::string &raw) {
        std::vector<std::shared_ptr<Regular>> vector(raw.size());
        auto j = 0;
        for (auto i = raw.cbegin(); i != raw.cend(); ({
            vector[j++] = rs(*i);
            i++;
        }));
        return std::make_shared<regular::linear::Concatenation>(std::move(vector));
    }

    std::shared_ptr<Regular> rk(const std::shared_ptr<Regular> &regular) {
        return std::make_shared<regular::KleeneStar>(regular);
    }
}
