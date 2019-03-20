#pragma once

#include <functional>
#include <list>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

#include "json/json.hh"

namespace regular {
    using Json=json::Json<double, wchar_t>;

    class RegExp : public std::enable_shared_from_this<RegExp> {
        bool _collapsed = false;
    public:
        static inline std::shared_ptr<RegExp> collapsed(const std::shared_ptr<RegExp> &r) {
            r->_collapsed = true;
            return r;
        }

        class Match : public std::enable_shared_from_this<Match> {
        public:
            const bool success;

            const std::wstring::const_iterator begin, end;

            explicit Match(
                    const bool &success,
                    const std::wstring::const_iterator &begin,
                    const std::wstring::const_iterator &end
            ) :
                    success(success),
                    begin(begin),
                    end(end) {}

            template<typename Derived>
            std::shared_ptr<Derived> as() {
                return std::static_pointer_cast<Derived>(shared_from_this());
            }

            virtual Json json() {
                std::wstring s;
                s.resize(std::size_t(end - begin));
                for (auto i = begin; i != end; ({
                    *(s.begin() + (i - begin)) = *i;
                    i++;
                }));

                return s;
            }
        };

        template<typename Derived>
        std::shared_ptr<Derived> as() {
            return std::static_pointer_cast<Derived>(shared_from_this());
        }

        std::shared_ptr<Match> match(
                const std::wstring::const_iterator &i,
                const std::wstring::const_iterator &i1
        ) {
            auto m = this->_match(i, i1);
            if (_collapsed) return std::make_shared<Match>(m->success, m->begin, m->end);
            else return m;
        }

    private:
        virtual std::shared_ptr<Match>
        _match(
                const std::wstring::const_iterator &,
                const std::wstring::const_iterator &
        ) = 0;
    };

    namespace regexp {
        class Empty : public RegExp {
            std::shared_ptr<RegExp::Match>
            _match(
                    const std::wstring::const_iterator &i,
                    const std::wstring::const_iterator &
            ) final { return std::make_shared<RegExp::Match>(true, i, i); }
        };

        template<typename Context= nullptr_t>
        class Singleton : public RegExp {
            std::shared_ptr<RegExp::Match>
            _match(
                    const std::wstring::const_iterator &i,
                    const std::wstring::const_iterator &i1
            ) final {
                return (i != i1 && _function(_context, *i))
                       ? std::make_shared<RegExp::Match>(true, i, i + 1)
                       : std::make_shared<RegExp::Match>(false, i, i);
            }

            const Context _context;
            const std::function<bool(const Context &, const wchar_t &)> _function;
        public:
            Singleton(
                    Context context,
                    std::function<bool(const Context &, const wchar_t &)> function
            ) : _context(std::move(context)),
                _function(std::move(function)) {}
        };

        class Linear : public RegExp {
        protected:
            std::list<std::pair<
                    std::wstring,
                    std::shared_ptr<RegExp>
            >> _list;
        public:

            std::shared_ptr<Linear> item(const std::shared_ptr<RegExp> &reg, std::wstring name = L"") {
                _list.emplace_back(std::make_pair(std::move(name), reg));
                return std::dynamic_pointer_cast<Linear>(shared_from_this());
            }
        };

        namespace linear {
            class Union : public Linear {
            public:
                class Match : public RegExp::Match {
                public:
                    const std::wstring key;
                    const std::shared_ptr<RegExp::Match> value;

                    Match(
                            const bool &success,
                            const std::wstring::const_iterator &begin,
                            const std::wstring::const_iterator &end,
                            std::wstring key,
                            std::shared_ptr<RegExp::Match> value
                    ) :
                            RegExp::Match(success, begin, end),
                            key(std::move(key)),
                            value(std::move(value)) {}

                    Json json() final { return value->json(); }
                };

            private:
                std::shared_ptr<RegExp::Match>
                _match(
                        const std::wstring::const_iterator &i,
                        const std::wstring::const_iterator &i1
                ) final {
                    auto m = std::make_shared<RegExp::Match>(false, i, i);
                    std::wstring key;
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

            class Intersection : public Linear {
                std::shared_ptr<RegExp::Match>
                _match(
                        const std::wstring::const_iterator &i,
                        const std::wstring::const_iterator &i1
                ) final {
                    bool success;
                    auto end = i;

                    for (auto j = _list.cbegin(); j != _list.cend(); ({
                        auto m = j->second->match(i, i1);
                        end = m->end;
                        if (m->success) {
                            success = true;
                            for (auto k = _list.cbegin(); k != _list.cend(); ({
                                if (k != j) {
                                    auto m1 = k->second->match(i, m->end);
                                    if (!(m1->success && m1->end == m->end)) {
                                        success = false;
                                        k = _list.cend();
                                    } else k++;
                                } else k++;
                            }));
                            if (success) j = _list.cend();
                            else j++;
                        } else {
                            success = false;
                            j = _list.cend();
                        }
                    }));

                    return std::make_shared<Match>(success, i, end);
                }
            };

            class Concatenation : public Linear {
            public:
                class Match : public RegExp::Match {
                public:
                    const std::list<std::shared_ptr<RegExp::Match>> list;
                    const std::unordered_map<std::wstring, std::shared_ptr<RegExp::Match>> map;

                    Match(
                            const bool &success,
                            const std::wstring::const_iterator &begin,
                            const std::wstring::const_iterator &end,
                            std::list<std::shared_ptr<RegExp::Match>> list,
                            std::unordered_map<std::wstring, std::shared_ptr<RegExp::Match>> map
                    ) :
                            RegExp::Match(success, begin, end),
                            list(std::move(list)),
                            map(std::move(map)) {}

                    Json json() final {
                        auto m = std::unordered_map<std::wstring, Json>();
                        for (auto i = map.cbegin(); i != map.cend(); ({
                            if (!i->first.empty()) m[i->first] = i->second->json();
                            i++;
                        }));
                        return m;
                    }
                };

            private:
                std::shared_ptr<RegExp::Match>
                _match(
                        const std::wstring::const_iterator &i0,
                        const std::wstring::const_iterator &i1
                ) final {
                    bool success = true;
                    auto end = i0;
                    std::list<std::shared_ptr<RegExp::Match>> list;
                    std::unordered_map<std::wstring, std::shared_ptr<RegExp::Match>> map;

                    auto i = i0;
                    for (auto j = _list.cbegin(); j != _list.cend(); ({
                        auto m = j->second->match(i, i1);
                        m->success ? ({
                            list.emplace_back(m);
                            if (!j->first.empty()) map[j->first] = m;
                            i = m->end;
                            j++;
                        }) : ({
                            success = false;
                            end = m->end;
                            j = _list.cend();
                        });
                    }));
                    if (success) end = i;

                    return std::make_shared<Match>(success, i0, end, std::move(list), std::move(map));
                }
            };
        }

        class Difference : public RegExp {
            std::shared_ptr<RegExp::Match>
            _match(
                    const std::wstring::const_iterator &i0,
                    const std::wstring::const_iterator &i1
            ) final {
                auto m = _global->match(i0, i1),
                        m1 = _local->match(i0, m->end);
                return std::make_shared<Match>(!(m1->success && m1->end == m->end), i0, m->end);
            }

            const std::shared_ptr<RegExp> _global, _local;
        public:
            explicit Difference(
                    std::shared_ptr<RegExp> global,
                    std::shared_ptr<RegExp> local
            ) : _global(std::move(global)),
                _local(std::move(local)) {}
        };

        class KleeneStar : public RegExp {
        public:
            class Match : public RegExp::Match {
            public:
                const std::list<std::shared_ptr<RegExp::Match>> list;

                Match(
                        const bool &success,
                        const std::wstring::const_iterator &begin,
                        const std::wstring::const_iterator &end,
                        std::list<std::shared_ptr<RegExp::Match>> &&list
                ) :
                        RegExp::Match(success, begin, end),
                        list(std::move(list)) {}

                Json json() final {
                    auto l = std::list<Json>();
                    for (auto i = list.cbegin(); i != list.cend(); ({
                        l.emplace_back((*i)->json());
                        i++;
                    }));
                    return l;
                }
            };

        private:
            std::shared_ptr<RegExp::Match>
            _match(
                    const std::wstring::const_iterator &i0,
                    const std::wstring::const_iterator &i1
            ) final {
                std::list<std::shared_ptr<RegExp::Match>> list;
                auto i = i0, end = i0;
                while (({
                    auto m = _repeat->match(i, i1);
                    m->success && (!_positive_width || m->end > i) ? ({
                        list.emplace_back(m);
                        i = m->end;
                        true;
                    }) : ({
                        end = i;
                        false;
                    });
                }));
                return std::make_shared<Match>(true, i0, end, std::move(list));
            }

            const std::shared_ptr<RegExp> _repeat;
            const bool _positive_width;
        public:
            explicit KleeneStar(std::shared_ptr<RegExp> repeat, const bool &positive_width) :
                    _repeat(std::move(repeat)),
                    _positive_width(positive_width) {}
        };
    }


    auto CLPSD = &RegExp::collapsed;

    using RM=typename RegExp::Match;

    std::shared_ptr<regexp::Empty> RN() {
        return std::make_shared<regexp::Empty>();
    }

    std::shared_ptr<regexp::Singleton<>> RS(const std::function<bool(const nullptr_t &, const wchar_t &)> &callback) {
        return std::make_shared<regexp::Singleton<>>(nullptr, callback);
    }

    std::shared_ptr<regexp::Singleton<bool>> RSA(const bool &any = true) {
        return std::make_shared<regexp::Singleton<bool>>(
                any,
                [&](const bool &any, const wchar_t &t) -> bool { return any; }
        );
    }

    namespace context {
        struct IsCharacter {
            const bool is;
            const wchar_t character;
        };
    }

    std::shared_ptr<regexp::Singleton<context::IsCharacter>> RSI(const wchar_t &c, const bool &is = true) {
        using namespace context;
        return std::make_shared<regexp::Singleton<IsCharacter>>(
                IsCharacter{is, c},
                [&](const IsCharacter &c, const wchar_t &t) -> bool {
                    return (!c.is) xor (t == c.character);
                }
        );
    }

    namespace context {
        struct InString {
            const bool in;
            const std::wstring string;
        };
    }

    std::shared_ptr<regexp::Singleton<context::InString>> RSI(const std::wstring &s, const bool &in = true) {
        using namespace context;
        return std::make_shared<regexp::Singleton<context::InString>>(
                InString{in, s},
                [&](const InString &c, const wchar_t &t) -> bool {
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
            const wchar_t inf;
            const wchar_t sup;
        };
    }

    std::shared_ptr<regexp::Singleton<context::Interval>> RSR(const wchar_t &inf, const wchar_t &sup) {
        using namespace context;
        return std::make_shared<regexp::Singleton<Interval>>(
                Interval{inf, sup},
                [&](const Interval &c, const wchar_t &t) -> bool { return t >= c.inf && t <= c.sup; }
        );
    }

    namespace context {
        using IntervalList=std::list<std::pair<wchar_t, wchar_t>>;
    }

    std::shared_ptr<regexp::Singleton<context::IntervalList>> RSRL(const std::list<std::pair<wchar_t, wchar_t>> &list) {
        using namespace context;
        return std::make_shared<regexp::Singleton<IntervalList >>(
                list,
                [&](const IntervalList &c, const wchar_t &t) -> bool {
                    for (auto i = c.cbegin(); i != c.cend(); ({
                        if (t >= i->first && t <= i->second) return true;
                        i++;
                    }));
                    return false;
                }
        );
    }

    template<typename Context>
    using RSM=typename regexp::Singleton<Context>::Match;

    std::shared_ptr<regexp::linear::Union> RU() {
        return std::make_shared<regexp::linear::Union>();
    }

    using RUM=typename regexp::linear::Union::Match;

    std::shared_ptr<regexp::linear::Intersection> RI() {
        return std::make_shared<regexp::linear::Intersection>();
    }

    std::shared_ptr<regexp::linear::Concatenation> RC() {
        return std::make_shared<regexp::linear::Concatenation>();
    }

    std::shared_ptr<regexp::linear::Concatenation> RC(const std::wstring &raw) {
        auto rc = RC();
        for (auto i = raw.cbegin(); i != raw.cend(); ({
            rc->item(RSI(*i));
            i++;
        }));
        return rc;
    }

    using RCM=typename regexp::linear::Concatenation::Match;

    std::shared_ptr<regexp::Difference>
    RK(const std::shared_ptr<RegExp> &global, const std::shared_ptr<RegExp> &local) {
        return std::make_shared<regexp::Difference>(global, local);
    }

    std::shared_ptr<regexp::KleeneStar>
    RK(
            const std::shared_ptr<RegExp> &repeat,
            const bool &positive_width = true
    ) { return std::make_shared<regexp::KleeneStar>(repeat, positive_width); }

    using RKM=typename regexp::KleeneStar::Match;
}
