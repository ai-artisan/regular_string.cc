#pragma once

#include "regular.h"

namespace regular {
    template<typename Character>
    struct Record {
        using String = typename CharacterTraits<Character>::String;
        using StringIterator = typename String::const_iterator;
        using PtrRecord = std::shared_ptr<Record>;
        using Children = std::list<std::pair<String, PtrRecord>>;
        using Reduced = std::unordered_map<String, std::list<PtrRecord>>;

        bool success;
        StringIterator begin, end;
        Children children;

        static Reduced reduce(const Children &children) {
            Reduced reduced;
            for (auto &&[tag, value]:children) reduced[tag].emplace_back(value);
            return reduced;
        }
    };
}