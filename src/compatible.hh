#pragma once

#include "hub.hh"

namespace regular {
    template<typename Character>
    struct compatible {
        ~compatible() = delete;

        using hub=regular::hub<Character>;

        static inline std::shared_ptr<typename hub::plct> p_om(const std::shared_ptr<typename hub::pt> &p) {
            return hub::plc({{Traits<Character>::string("First"), p},
                             {Traits<Character>::string("Rest"),  hub::pk(p)}});
        }

        static inline std::shared_ptr<typename hub::plut> p_oz(const std::shared_ptr<typename hub::pt> &p) {
            return hub::plu({{Traits<Character>::string("One"),  p},
                             {Traits<Character>::string("Zero"), hub::po()}});
        }

        static inline std::shared_ptr<typename hub::pst> p_dgt() {
            return hub::ps(48, 57);
        }

        static inline std::shared_ptr<typename hub::pst> p_abdu() {
            return hub::psu({hub::ps(97, 122), hub::ps(65, 90), p_dgt(), hub::ps(95)});
        }

        static inline std::shared_ptr<typename hub::pst> p_ws() {
            return hub::ps(Traits<Character>::string(" \f\n\r\t\v"));
        }
    };

    namespace shortcut {
        namespace narrow {
            inline std::shared_ptr<plct> p_om(const std::shared_ptr<pt> &p) {
                return compatible<char>::p_om(p);
            }

            inline std::shared_ptr<plut> p_oz(const std::shared_ptr<pt> &p) {
                return compatible<char>::p_oz(p);
            }

            const auto p_dgt = compatible<char>::p_dgt();

            const auto p_abdu = compatible<char>::p_abdu();

            const auto p_ws = compatible<char>::p_ws();
        }
        namespace wide {
            inline std::shared_ptr<wplct> wp_om(const std::shared_ptr<wpt> &p) {
                return compatible<wchar_t>::p_om(p);
            }

            inline std::shared_ptr<wplut> wp_oz(const std::shared_ptr<wpt> &p) {
                return compatible<wchar_t>::p_oz(p);
            }

            const auto wp_dgt = compatible<wchar_t>::p_dgt();

            const auto wp_abdu = compatible<wchar_t>::p_abdu();

            const auto wp_ws = compatible<wchar_t>::p_ws();
        }
    }
}