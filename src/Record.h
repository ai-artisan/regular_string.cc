#pragma once

#include "regular.h"

namespace regular {
    template<typename Character>
    struct Record : std::enable_shared_from_this<Record<Character>> {
        using PtrRecord = std::shared_ptr<Record>;
        using String = typename CharacterTraits<Character>::String;
        using StringIterator = typename String::const_iterator;

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

            const std::size_t index;
            const PtrRecord value;

            LinearSome(const StringIterator &begin, const StringIterator &direct_end, const StringIterator &greedy_end,
                       const std::size_t &index, const PtrRecord &value) :
                    Record<Character>(begin, direct_end, greedy_end),
                    index(index), value(value) {}
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

            const Vector vector;

            LinearEvery(const StringIterator &begin, const StringIterator &direct_end, const StringIterator &greedy_end,
                        Vector vector) :
                    Record<Character>(begin, direct_end, greedy_end),
                    vector(std::move(vector)) {}
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

        template<typename Character>
        struct Mark : Record<Character> {
            using PtrRecord = typename Record<Character>::PtrRecord;
            using String = typename Record<Character>::String;
            using StringIterator = typename Record<Character>::StringIterator;

            const String tag;
            const PtrRecord value;

            Mark(const StringIterator &begin, const StringIterator &direct_end, const StringIterator &greedy_end,
                 String tag, const PtrRecord &value) :
                    Record<Character>(begin, direct_end, greedy_end),
                    tag(std::move(tag)), value(value) {}
        };

        namespace mark {
            template<typename Character>
            struct List : Mark<Character> {
                using PtrRecord = typename Record<Character>::PtrRecord;
                using String = typename Record<Character>::String;
                using StringIterator = typename Record<Character>::StringIterator;

                List(const StringIterator &begin, const StringIterator &direct_end, const StringIterator &greedy_end,
                     String tag, const PtrRecord &value) :
                        Mark<Character>(begin, direct_end, greedy_end,
                                        std::move(tag), value) {}
            };

            template<typename Character>
            struct Dict : Mark<Character> {
                using PtrRecord = typename Record<Character>::PtrRecord;
                using String = typename Record<Character>::String;
                using StringIterator = typename Record<Character>::StringIterator;

                Dict(const StringIterator &begin, const StringIterator &direct_end, const StringIterator &greedy_end,
                     String tag, const PtrRecord &value) :
                        Mark<Character>(begin, direct_end, greedy_end,
                                        std::move(tag), value) {}
            };
        }
    }
}