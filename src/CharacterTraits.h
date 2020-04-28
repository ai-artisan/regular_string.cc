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

        using String = std::string;

        static std::string string(const std::string &s) { return s; }

        static char character(const char &c) { return c; }
    };

    template<>
    struct CharacterTraits<wchar_t> {
        ~CharacterTraits() = delete;

        using String = std::wstring;

        static std::wstring string(const std::string &s) {
            return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(s);
        }

        static wchar_t character(const char &c) {
            return string(std::string(1, c)).at(0);
        }
    };
}