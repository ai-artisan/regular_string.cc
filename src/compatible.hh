#pragma once

#include "hub.hh"

namespace regular {
    template<typename Character>
    struct compatible {
        ~compatible() = delete;

        using hub=regular::hub<Character>;

        static std::shared_ptr<typename hub::pct> pk_plus(const std::shared_ptr<typename hub::pt> &p) {
            return hub::pc(hub::plc({p, hub::pk(p)}), [](typename hub::pct::Matched &m) -> std::shared_ptr<typename hub::rt> {
                auto &[b, r]=m;
                auto &lc = r->template as<typename hub::rlet>()->list;
                auto &rp = lc.front();
                std::shared_ptr<typename hub::rkt> rk;
                if (b) rk = lc.back()->template as<typename hub::rkt>();
                else {
                    rk = std::make_shared<typename hub::rkt>();
                    rk->end = rp->end;
                }
                rk->begin = rp->begin;
                auto &lp = rk->list;
                lp.emplace(lp.cbegin(), rp);
                return rk;
            });
        }
    };

    namespace shortcut {
        namespace narrow {
            inline std::shared_ptr<hub<char>::pct> pk_plus(const std::shared_ptr<typename hub<char>::pt> &p) {
                return compatible<char>::pk_plus(p);
            }
        }
        namespace wide {
            inline std::shared_ptr<hub<wchar_t>::pct> wpk_plus(const std::shared_ptr<typename hub<wchar_t>::pt> &p) {
                return compatible<wchar_t>::pk_plus(p);
            }
        }
    }
}