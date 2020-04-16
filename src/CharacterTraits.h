#pragma once

#include "regular.h"

namespace regular {
    template<typename...>
    struct CharacterTraits {
        ~CharacterTraits() = delete;
    };

    template<>
    struct CharacterTraits<char> {
        ~CharacterTraits() = delete;

        using String=std::string;

        static inline String string(const std::string &s) {
            return s;
        }
    };

    template<>
    struct CharacterTraits<wchar_t> {
        ~CharacterTraits() = delete;

        using String=std::wstring;

        static inline String string(const std::string &s) {
            return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(s);
        }
    };
}