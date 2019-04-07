#include "regular.h"

namespace regular {
    namespace record {
        template<typename Character>
        typename Traits<Character>::String::const_iterator Naive<Character>::end() const {
            return model[1];
        }

        template<typename Character>
        typename Traits<Character>::String::const_iterator Union<Character>::end() const {
            return model.second->end();
        }

        template<typename Character>
        typename Traits<Character>::String::const_iterator Concatenation<Character>::end() const {
            return model[1]->end();
        }

        template<typename Character>
        typename Traits<Character>::String::const_iterator KleeneClosure<Character>::end() const {
            return model.back()->end();
        }
    }

    namespace pattern {
        template<typename Character>
        std::pair<bool, std::shared_ptr<Record < Character>>>

        inline Empty<Character>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            return std::pair{true, std::make_shared<record::Naive<Character>>(std::array{begin, begin})};
        }

        template<typename Character, typename Context>
        std::pair<bool, std::shared_ptr<Record < Character>>>

        Singleton<Character, Context>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            const auto &[describe, context]=model;
            return (begin != end && describe(*begin, context))
                   ? std::pair{true, std::make_shared<record::Naive<Character>>(std::array{begin, std::next(begin)})}
                   : std::pair{false, std::make_shared<record::Naive<Character>>(std::array{begin, begin})};
        }

        namespace binary {
            template<typename Character>
            std::pair<bool, std::shared_ptr<Record < Character>>>

            Union<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                auto pair = this->model[0]->match(begin, end);
                if (pair.first) return std::pair{true, std::make_shared<record::Union<Character>>(std::pair{0, pair.second})};
                else {
                    pair = this->model[1]->match(begin, end);
                    if (pair.first) return std::pair{true, std::make_shared<record::Union<Character>>(std::pair{1, pair.second})};
                    else return std::pair{false, nullptr};
                }
            }

            template<typename Character>
            std::pair<bool, std::shared_ptr<Record < Character>>>

            Concatenation<Character>::match(
                    const typename Traits<Character>::String::const_iterator &begin,
                    const typename Traits<Character>::String::const_iterator &end
            ) const {
                bool success = false;
                std::array<std::shared_ptr<Record<Character>>, 2> array;
                auto pair = this->model[0]->match(begin, end);
                if (pair.first) {
                    array[0] = pair.second;
                    pair = this->model[1]->match(begin, end);
                    if (pair.first) {
                        array[1] = pair.second;
                        success = true;
                    }
                }
                return std::pair{success, std::make_shared<record::Concatenation<Character>>(std::move(array))};
            }
        }

        template<typename Character>
        std::pair<bool, std::shared_ptr<Record < Character>>>

        KleeneClosure<Character>::match(
                const typename Traits<Character>::String::const_iterator &begin,
                const typename Traits<Character>::String::const_iterator &end
        ) const {
            std::list<std::shared_ptr<Record<Character>>> list;
            auto i = begin, j = end;
            while (({
                auto[success, record] = model->match(i, end);
                success && (record->end() > i) ? ({
                    list.emplace_back(record);
                    i = record->end();
                    true;
                }) : ({
                    j = i;
                    false;
                });
            }));
            return std::pair{true, std::make_shared<record::KleeneClosure<Character>>(std::move(list))};
        }
    }
}


//
//template<typename CharacterType>
//template<typename ContextType>
//std::shared_ptr<typename Regular<CharacterType>::Pattern::Record> Regular<CharacterType>::Pattern::Singleton<ContextType>::match(const typename String::const_iterator &begin, const typename String::const_iterator &end) {
//    return (begin != end && describe(*begin, context))
//           ? std::make_shared<Record>(true, begin, std::next(begin))
//           : std::make_shared<Record>(false, begin, begin);
//}



//namespace regular {

//
//    namespace pattern {
//
//        template<typename CharacterType>
//        Linear<CharacterType>::~Linear() = default;
//
//        template<typename CharacterType>
//        Linear<CharacterType>::Linear(decltype(sequence) &&sequence):
//                sequence(std::move(sequence)) {}
//
//        namespace linear {
//            template<typename CharacterType>
//            Union<CharacterType>::Record::Record(decltype(key) &&key, decltype(value) &&value):
//                    key(std::move(key)),
//                    value(std::move(value)) {}
//
//            template<typename CharacterType>
//            inline typename Union<CharacterType>::Json Union<CharacterType>::Record::json() {
//                return value->json();
//            }
//
//            template<typename CharacterType>
//            std::shared_ptr<typename Union<CharacterType>::Record> Union<CharacterType>::match(const typename Union::String::const_iterator &begin, const typename Union::String::const_iterator &end) {
//                auto m = std::make_shared<typename Pattern<CharacterType>::Match>(false, begin, begin);
//                std::wstring key;
//                for (auto j = this->sequence.cbegin(); j != this->sequence.cend(); ({
//                    m = j->second->match(begin, end);
//                    if (m->success) {
//                        key = j->first;
//                        j = this->sequence.cend();
//                    } else j++;
//                }));
//                return std::make_shared<Record>(m->success, begin, m->end, key, m);
//            }
//        }
//    }
//}


