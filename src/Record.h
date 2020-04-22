#pragma once

#include "regular.h"

namespace regular {
    template<typename Character>
    struct Record : std::enable_shared_from_this<Record<Character>> {
        using PtrRecord = std::shared_ptr<Record>;
        using StringIterator = typename CharacterTraits<Character>::String::const_iterator;

        const StringIterator begin, direct_end, greedy_end;

        Record(const StringIterator &begin, const StringIterator &direct_end, const StringIterator &greedy_end) :
                begin(begin), direct_end(direct_end), greedy_end(greedy_end) {}

        virtual ~Record() = default;

        template<typename Derived>
        std::shared_ptr<Derived> as() const {
            return std::dynamic_pointer_cast<Derived>(const_cast<Record<Character> *>(this)->shared_from_this());
        }
    };

    namespace record {
        template<typename Character>
        struct Some : Record<Character> {
            using PtrRecord = typename Record<Character>::PtrRecord;
            using StringIterator = typename Record<Character>::StringIterator;

            const bool index;
            const PtrRecord value;

            Some(const StringIterator &begin, const StringIterator &direct_end, const StringIterator &greedy_end,
                 const bool &index, const PtrRecord &value) :
                    Record<Character>(begin, direct_end, greedy_end),
                    index(index), value(value) {}
        };

        template<typename Character>
        struct LinearSome : Record<Character> {
            using PtrRecord = typename Record<Character>::PtrRecord;
            using StringIterator = typename Record<Character>::StringIterator;
            using Key = typename CharacterTraits<Character>::String;

            const std::size_t index;
            const Key key;
            const PtrRecord value;

            LinearSome(const StringIterator &begin, const StringIterator &direct_end, const StringIterator &greedy_end,
                       const std::size_t &index, Key key, const PtrRecord &value) :
                    Record<Character>(begin, direct_end, greedy_end),
                    index(index), key(std::move(key)), value(value) {}
        };

        template<typename Character>
        struct Every : Record<Character> {
            using PtrRecord = typename Record<Character>::PtrRecord;
            using StringIterator = typename Record<Character>::StringIterator;

            const PtrRecord first, second;

            Every(const StringIterator &begin, const StringIterator &direct_end, const StringIterator &greedy_end,
                  const PtrRecord &first, const PtrRecord &second) :
                    Record<Character>(begin, direct_end, greedy_end),
                    first(first), second(second) {}
        };

        template<typename Character>
        struct LinearEvery : Record<Character> {
            using PtrRecord = typename Record<Character>::PtrRecord;
            using StringIterator = typename Record<Character>::StringIterator;
            using Vector = std::vector<PtrRecord>;
            using Map = std::unordered_map<typename CharacterTraits<Character>::String, PtrRecord>;

            const Vector vector;
            const Map map;

            LinearEvery(const StringIterator &begin, const StringIterator &direct_end, const StringIterator &greedy_end,
                        Vector vector, Map map) :
                    Record<Character>(begin, direct_end, greedy_end),
                    vector(std::move(vector)), map(std::move(map)) {}
        };

        template<typename Character>
        struct Greedy : Record<Character> {
            using PtrRecord = typename Record<Character>::PtrRecord;
            using StringIterator = typename Record<Character>::StringIterator;
            using List = std::list<PtrRecord>;

            const List list;

            Greedy(const StringIterator &begin, const StringIterator &direct_end, const StringIterator &greedy_end,
                   List list) :
                    Record<Character>(begin, direct_end, greedy_end),
                    list(std::move(list)) {}
        };
    }
}