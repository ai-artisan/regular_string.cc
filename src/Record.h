#pragma once

#include "regular.h"

namespace regular {
    template<typename Character>
    struct Record {
        using String = typename CharacterTraits<Character>::String;
        using StringIterator = typename String::const_iterator;
        using PtrRecord = std::shared_ptr<Record>;
        using Children = std::list<std::pair<String, PtrRecord>>;

        const bool success;
        const StringIterator begin, end;
        const Children children;

        Record(const bool &success,
               const StringIterator &begin, const StringIterator &end,
               Children children = {}) :
                success(success),
                begin(begin), end(end),
                children(std::move(children)) {}

        using Reduced = std::unordered_map<String, std::list<PtrRecord>>;

        static Reduced reduce(const Children &children) {
            Reduced reduced;
            for (auto &&[tag, value]:children) reduced[tag].emplace_back(value);
            return reduced;
        }
    };
}