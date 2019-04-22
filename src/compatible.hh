#pragma once

#include "hub.hh"

namespace regular {
    template<typename Character>
    struct compatible {
        ~compatible() = delete;

        using hub=regular::hub<Character>;

        static inline std::shared_ptr<typename hub::plct> plc_om(const std::shared_ptr<typename hub::pt> &p) {
            return hub::plc({{Traits<Character>::string("First"), p},
                             {Traits<Character>::string("Rest"),  hub::pk(p)}});
        }

        static inline std::shared_ptr<typename hub::plut> plu_oz(const std::shared_ptr<typename hub::pt> &p) {
            return hub::plu({{Traits<Character>::string("One"),  p},
                             {Traits<Character>::string("Zero"), hub::po()}});
        }

        static inline std::shared_ptr<typename hub::pst> ps_dgt() {
            return hub::ps(48, 57);
        }

        static inline std::shared_ptr<typename hub::pst> ps_abdu() {
            return hub::psu({hub::ps(97, 122), hub::ps(65, 90), ps_dgt(), hub::ps(95)});
        }

        static inline std::shared_ptr<typename hub::pst> ps_ws() {
            return hub::ps(Traits<Character>::string(" \f\n\r\t\v"));
        }
    };

    namespace shortcut {
        namespace narrow {
            inline std::shared_ptr<plct> plc_om(const std::shared_ptr<pt> &p) {
                return compatible<char>::plc_om(p);
            }

            inline std::shared_ptr<plut> plu_oz(const std::shared_ptr<pt> &p) {
                return compatible<char>::plu_oz(p);
            }

            const auto ps_dgt = compatible<char>::ps_dgt();

            const auto ps_abdu = compatible<char>::ps_abdu();

            const auto ps_ws = compatible<char>::ps_ws();
        }
        namespace wide {
            inline std::shared_ptr<wplct> wplc_om(const std::shared_ptr<wpt> &p) {
                return compatible<wchar_t>::plc_om(p);
            }

            inline std::shared_ptr<wplut> wplu_oz(const std::shared_ptr<wpt> &p) {
                return compatible<wchar_t>::plu_oz(p);
            }

            const auto wps_dgt = compatible<wchar_t>::ps_dgt();

            const auto wps_abdu = compatible<wchar_t>::ps_abdu();

            const auto wps_ws = compatible<wchar_t>::ps_ws();
        }
    }
}