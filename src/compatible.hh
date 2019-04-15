#pragma once

#include "regular.h"

namespace regular {
    template<typename Character>
    struct compatible {
        ~compatible() = delete;

        using hub=regular::hub<Character>;

        static std::shared_ptr<typename hub::pct> pk_plus(const std::shared_ptr<typename hub::pt> &p) {
            return hub::pc(hub::plc({p, hub::pk(p)}), [](std::shared_ptr<typename hub::rt> &r) -> std::shared_ptr<typename hub::rt> {
                return r;
            });
        }
    };
}