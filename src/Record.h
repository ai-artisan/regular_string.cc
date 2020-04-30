#pragma once

#include "regular.h"

namespace regular {
    template<typename Character>
    struct Record : std::enable_shared_from_this<Record<Character>> {
        using PtrRecord = std::shared_ptr<Record>;
        using String = typename CharacterTraits<Character>::String;
        using StringIterator = typename String::const_iterator;

        static auto collapse(const PtrRecord &record) {
            return std::make_shared<Record<Character>>(record->begin, record->direct_end, record->greedy_end);
        }

        const StringIterator begin, direct_end, greedy_end;

        Record(const StringIterator &begin, const StringIterator &direct_end, const StringIterator &greedy_end) :
                begin(begin), direct_end(direct_end), greedy_end(greedy_end) {}

        virtual ~Record() = default;

        template<typename Derived>
        std::shared_ptr<Derived> as() const {
            return std::dynamic_pointer_cast<Derived>(const_cast<Record *>(this)->shared_from_this());
        }

        using ListExtracted = std::list<std::pair<String, PtrRecord>>;
        using MapReduced = std::unordered_map<String, std::list<PtrRecord>>;

        virtual void extract(ListExtracted &) const {}

        static ListExtracted extract(const std::shared_ptr<Record> &record) {
            ListExtracted list;
            record->extract(list);
            return list;
        }

        static MapReduced reduce(const ListExtracted &list) {
            MapReduced map;
            for (auto &&[key, value]:list) {
                auto i = map.find(key);
                if (i == map.cend()) map.insert(std::make_pair(key, std::list<PtrRecord>{value}));
                else i->second.emplace_back(value);
            }
            return map;
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

            using ListExtracted = typename Record<Character>::ListExtracted;

            void extract(ListExtracted &list) const final {
                if (value) value->extract(list);
            }
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

            using ListExtracted = typename Record<Character>::ListExtracted;

            void extract(ListExtracted &list) const final {
                if (value) value->extract(list);
            }
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

            using ListExtracted = typename Record<Character>::ListExtracted;

            void extract(ListExtracted &list) const final {
                if (first) first->extract(list);
                if (second) second->extract(list);
            }
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

            using ListExtracted = typename Record<Character>::ListExtracted;

            void extract(ListExtracted &list) const final {
                for (auto &&item:vector) if (item) item->extract(list);
            }
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

            using ListExtracted = typename Record<Character>::ListExtracted;

            void extract(ListExtracted &list_) const final {
                for (auto &&item:list) if (item) item->extract(list_);
            }
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

            using ListExtracted = typename Record<Character>::ListExtracted;

            void extract(ListExtracted &list) const final {
                list.emplace_back(std::make_pair(tag, value));
            }
        };
    }
}