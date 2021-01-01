#include "regular.h"

namespace regular {
    template<typename Character>
    std::list<typename CharacterTraits<Character>::String> split(const typename CharacterTraits<Character>::String &s, const std::shared_ptr<regular::Pattern<Character>> &p_seq) {
        using String = typename CharacterTraits<Character>::String;

        std::list<String> l;

        auto i = s.cbegin(), j = i;
        while (j == s.cend() ? ({
            l.template emplace_back(String(i, j));
            false;
        }) : ({
            auto r = p_seq->match(j, s.cend());
            if (r->success) {
                l.template emplace_back(String(i, j));
                j = i = r->end;
            } else j++;
            true;
        }));

        return l;
    }
}