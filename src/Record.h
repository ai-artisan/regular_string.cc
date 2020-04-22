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

        std::list<std::pair<String, PtrRecord>> query(const std::list<String> &sl) const {
            std::unordered_map<String, std::nullptr_t> map;
            for (auto &&s:sl) map[s] = nullptr;
            std::list<std::pair<String, PtrRecord>> pl;
            this->query(map, pl);
            return pl;
        }

        virtual inline void query(std::unordered_map<String, std::nullptr_t> &map, std::list<std::pair<String, PtrRecord>> &pl) const {}
    };

    namespace record {
        template<typename Character>
        struct Some : Record<Character> {
            using PtrRecord = typename Record<Character>::PtrRecord;
            using String = typename Record<Character>::String;
            using StringIterator = typename Record<Character>::StringIterator;

            const bool index;
            const PtrRecord value;

            Some(const StringIterator &begin, const StringIterator &direct_end, const StringIterator &greedy_end,
                 const bool &index, const PtrRecord &value) :
                    Record<Character>(begin, direct_end, greedy_end),
                    index(index), value(value) {}

            inline void query(std::unordered_map<String, std::nullptr_t> &map, std::list<std::pair<String, PtrRecord>> &pl) const final {
                value->query(map, pl);
            }
        };

        template<typename Character>
        struct LinearSome : Record<Character> {
            using PtrRecord = typename Record<Character>::PtrRecord;
            using String = typename Record<Character>::String;
            using StringIterator = typename Record<Character>::StringIterator;
            using Key = String;

            const std::size_t index;
            const Key key;
            const PtrRecord value;

            LinearSome(const StringIterator &begin, const StringIterator &direct_end, const StringIterator &greedy_end,
                       const std::size_t &index, Key key, const PtrRecord &value) :
                    Record<Character>(begin, direct_end, greedy_end),
                    index(index), key(std::move(key)), value(value) {}

            void query(std::unordered_map<String, std::nullptr_t> &map, std::list<std::pair<String, PtrRecord>> &pl) const final {
                if (map.find(key) != map.cend()) pl.emplace_back(std::make_pair(key, value));
                else value->query(map, pl);
            }
        };

        template<typename Character>
        struct Every : Record<Character> {
            using PtrRecord = typename Record<Character>::PtrRecord;
            using String = typename Record<Character>::String;
            using StringIterator = typename Record<Character>::StringIterator;

            const PtrRecord first, second;

            Every(const StringIterator &begin, const StringIterator &direct_end, const StringIterator &greedy_end,
                  const PtrRecord &first, const PtrRecord &second) :
                    Record<Character>(begin, direct_end, greedy_end),
                    first(first), second(second) {}

            inline void query(std::unordered_map<String, std::nullptr_t> &map, std::list<std::pair<String, PtrRecord>> &pl) const final {
                first->query(map, pl);
                second->query(map, pl);
            }
        };

        template<typename Character>
        struct LinearEvery : Record<Character> {
            using PtrRecord = typename Record<Character>::PtrRecord;
            using String = typename Record<Character>::String;
            using StringIterator = typename Record<Character>::StringIterator;
            using Vector = std::vector<PtrRecord>;
            using Map = std::unordered_map<String, PtrRecord>;

            const Vector vector;
            const Map map;
            const std::unordered_map<PtrRecord, String> item2key;

            LinearEvery(const StringIterator &begin, const StringIterator &direct_end, const StringIterator &greedy_end,
                        Vector vector, Map map) :
                    Record<Character>(begin, direct_end, greedy_end),
                    vector(std::move(vector)), map(std::move(map)), item2key([&]() {
                std::unordered_map<PtrRecord, String> item2key;
                for (auto &&[key, item]:this->map)item2key[item] = key;
                return item2key;
            }()) {}

            void query(std::unordered_map<String, std::nullptr_t> &map, std::list<std::pair<String, PtrRecord>> &pl) const final {
                for (auto &&item:vector) {
                    auto i = item2key.find(item);
                    if (i == item2key.cend() || map.find(i->second) == map.cend()) item->query(map, pl);
                    else pl.emplace_back(std::make_pair(i->second, item));
                }
            }
        };

        template<typename Character>
        struct Greedy : Record<Character> {
            using PtrRecord = typename Record<Character>::PtrRecord;
            using String = typename Record<Character>::String;
            using StringIterator = typename Record<Character>::StringIterator;
            using List = std::list<PtrRecord>;

            const List list;

            Greedy(const StringIterator &begin, const StringIterator &direct_end, const StringIterator &greedy_end,
                   List list) :
                    Record<Character>(begin, direct_end, greedy_end),
                    list(std::move(list)) {}

            inline void query(std::unordered_map<String, std::nullptr_t> &map, std::list<std::pair<String, PtrRecord>> &pl) const final {
                for (auto &&item:list) item->query(map, pl);
            }
        };
    }
}