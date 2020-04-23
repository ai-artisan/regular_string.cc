#pragma once

#include "regular.h"

namespace regular {
    template<typename Character>
    struct Data : std::enable_shared_from_this<Data<Character>> {
        using String = typename CharacterTraits<Character>::String;
        using PtrRecord = std::shared_ptr<Record < Character>>;

        const String tag;
        const PtrRecord record;

        Data(String tag, const PtrRecord &record) : tag(std::move(tag)), record(record) {}

        template<typename Derived>
        std::shared_ptr<Derived> as() const {
            return std::dynamic_pointer_cast<Derived>(const_cast<Data *>(this)->shared_from_this());
        }
    };

    namespace data {
        template<typename Character>
        struct List : Data<Character> {
            using String = typename CharacterTraits<Character>::String;
            using PtrRecord = std::shared_ptr<Record < Character>>;
            using PtrData = std::shared_ptr<Data<Character>>;
            using List_ = std::list<PtrData>;

            const List_ list;

            List(String tag, const PtrRecord &record, List_ list) : Data<Character>(std::move(tag), record),
                                                                    list(std::move(list)) {}
        };

        template<typename Character>
        struct Dict : Data<Character> {
            using String = typename CharacterTraits<Character>::String;
            using PtrRecord = std::shared_ptr<Record < Character>>;
            using PtrData = std::shared_ptr<Data<Character>>;
            using Dict_ = std::unordered_map<String, PtrData>;

            const Dict_ dict;

            Dict(String tag, const PtrRecord &record, Dict_ dict) : Data<Character>(std::move(tag), record),
                                                                    dict(std::move(dict)) {}
        };
    }
}