#include "regular.h"

namespace regular {
    template<typename CharacterType>
    Traits<CharacterType>::~Traits() = default;

    regular::Traits<char>::~Traits() = default;

    regular::Traits<wchar_t>::~Traits() = default;

    template<typename CharacterType>
    Pattern<CharacterType>::Record::Record(const bool &success, const typename Pattern::String::const_iterator &begin, const typename Pattern::String::const_iterator &end)
            :success(success),
             begin(begin),
             end(end) {}

    template<typename CharacterType>
    inline typename Pattern<CharacterType>::Json Pattern<CharacterType>::Record::json() {
        return typename Pattern::String(begin, end);
    }

    template<typename CharacterType>
    template<typename DerivedType>
    inline std::shared_ptr<DerivedType> Pattern<CharacterType>::Record::as() {
        std::static_pointer_cast<DerivedType>(this->shared_from_this());
    }

    namespace pattern {
        template<typename CharacterType>
        inline std::shared_ptr<typename Null<CharacterType>::Record> Null<CharacterType>::match(const typename Null::String::const_iterator &begin, const typename Null::String::const_iterator &end) {
            typename Null::Record r{true, begin, begin};
            return std::make_shared<typename Null::Record>(true, begin, begin);
        }

        template<typename CharacterType, typename ContextType>
        inline std::shared_ptr<typename Singleton<CharacterType, ContextType>::Record> Singleton<CharacterType, ContextType>::match(const typename Singleton::String::const_iterator &begin, const typename Singleton::String::const_iterator &end) {
            return (begin != end && describe(*begin, context))
                   ? std::make_shared<typename Singleton::Record>(true, begin, std::next(begin))
                   : std::make_shared<typename Singleton::Record>(false, begin, begin);
        }

        template<typename CharacterType, typename ContextType>
        Singleton<CharacterType, ContextType>::Singleton(decltype(describe) &&describe, ContextType &&context):
                describe(std::move(describe)),
                context(std::move(context)) {}

        template<typename CharacterType>
        Linear<CharacterType>::~Linear() = default;

        template<typename CharacterType>
        Linear<CharacterType>::Linear(decltype(sequence) &&sequence):
                sequence(std::move(sequence)) {}
    }
}