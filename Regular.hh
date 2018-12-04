#pragma once

#include <functional>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

namespace regular_string {
    class Regular {
    public:
        virtual ~Regular() = 0;

        class Match {
        public:
            const bool success;
            const std::string::const_iterator end;

            explicit Match(bool success, std::string::const_iterator end) :
                    success(std::move(success)),
                    end(std::move(end)) {}
        };

        virtual std::shared_ptr<Match>
        match(const std::string::const_iterator &, const std::string::const_iterator &) = 0;
    };

    namespace regular {
        class Empty {
        public:
            std::shared_ptr<Regular::Match>
            match(const std::string::const_iterator &i, const std::string::const_iterator &) {
                return std::make_shared<Regular::Match>(true, i);
            }
        };

        class Singleton {
            std::function<bool(const std::string::const_iterator &)> _function;
        public:
            explicit Singleton(decltype(_function) function) {
                _function = std::move(function);
            }

            std::shared_ptr<Regular::Match>
            match(const std::string::const_iterator &i, const std::string::const_iterator &end) {
                return (i != end && _function(i)) ? std::make_shared<Regular::Match>(true, std::next(i))
                                                  : std::make_shared<Regular::Match>(false, i);
            }
        };

        class Linear {
        public:
            struct Item {
                const std::string key;
                const std::shared_ptr<Regular> regular;

                Item(std::string key, std::shared_ptr<Regular> regular) :
                        key(std::move(key)),
                        regular(std::move(regular)) {}

                Item(std::shared_ptr<Regular> regular) : Item("", std::move(regular)) {}
            };

        protected:
            std::vector<std::shared_ptr<Regular>> _vector;
            std::unordered_map<std::size_t, std::string> _map;
        public:
            explicit Linear(const std::vector<Item> &vector) {
                _vector.resize(vector.size());
                for (auto i = 0; i < vector.size(); ({
                    _vector[i] = vector[i].regular;
                    _map.insert(std::make_pair(i, vector[i].key));
                    i++;
                }));
            }
        };

        namespace linear {
            class Union : public Linear {
            public:
                explicit Union(const std::vector<Item> &vector) : Linear(vector) {}

                class Match : public Regular::Match {
                public:
                    const std::size_t index;
                    const std::string key;

                    Match(std::string::const_iterator end, const std::size_t &index, std::string key) :
                            Regular::Match(true, end),
                            index(index),
                            key(std::move(key)) {}
                };

                std::shared_ptr<Regular::Match>
                match(const std::string::const_iterator &i, const std::string::const_iterator &end) {
                    auto m = std::make_shared<Regular::Match>(false, i);
                    for (auto j = 0; j < _vector.size(); ({
                        m = _vector[j]->match(i, end);
                        if (m->success) return std::make_shared<Match>(m->end, j, _map[j]);
                        else j++;
                    }));
                    return m;
                }
            };

            class Concatenation : public Linear {
            public:
                explicit Concatenation(const std::vector<Item> &vector) : Linear(vector) {}

                class Match : public Regular::Match {
                public:
                    const std::vector<std::shared_ptr<Regular::Match>> vector;
                    const std::unordered_map<std::string, std::shared_ptr<Regular::Match>> map;

                    Match(
                            std::string::const_iterator end,
                            std::vector<std::shared_ptr<Regular::Match>> &&vector,
                            std::unordered_map<std::string, std::shared_ptr<Regular::Match>> &&map
                    ) :
                            Regular::Match(true, end),
                            vector(std::move(vector)),
                            map(std::move(map)) {}
                };

                std::shared_ptr<Regular::Match>
                match(const std::string::const_iterator &i, const std::string::const_iterator &end) {

                }
            };
        }

        class KleeneStar {
            const std::shared_ptr<Regular> _value;
        public:

        };
    }
}
#pragma clang diagnostic pop