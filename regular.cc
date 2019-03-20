#include "regular.h"

namespace regular {
    template<typename Character>
    Pattern<Character>::Record::Record(const bool &success, const typename Traits<Character>::String::const_iterator &begin, const typename Traits<Character>::String::const_iterator &end)
            :success(success),
             begin(begin),
             end(end) {}

    template<typename Character>
    inline typename Traits<Character>::Json Pattern<Character>::Record::json() {
        return typename Traits<Character>::String(begin, end);
    }

    template<typename Character>
    template<typename Derived>
    inline std::shared_ptr<Derived> Pattern<Character>::Record::as() {
        std::static_pointer_cast<Derived>(this->shared_from_this());
    }

    namespace pattern {
        template<typename Character>
        inline std::shared_ptr<typename Null<Character>::Record> Null<Character>::match(const typename Traits<Character>::String::const_iterator &begin, const typename Traits<Character>::String::const_iterator &end) {
            Record r{true, begin, begin};
            return std::make_shared<Record>(true, begin, begin);
        }

        template<typename Character, typename Context>
        inline std::shared_ptr<typename Singleton<Character, Context>::Record> Singleton<Character, Context>::match(const typename Traits<Character>::String::const_iterator &begin, const typename Traits<Character>::String::const_iterator &end) {
            return (begin != end && describe(*begin, context))
                   ? std::make_shared<Record>(true, begin, std::next(begin))
                   : std::make_shared<Record>(false, begin, begin);
        }

        template<typename Character, typename Context>
        Singleton<Character, Context>::Singleton(const decltype(describe) &describe, const Context &context):
                describe(describe),
                context(context) {}
    }
}