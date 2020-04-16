#ifdef REGULAR_CHARACTER_TYPE

using rt=regular::Record<REGULAR_CHARACTER_TYPE>;
using rut=regular::record::Unary<REGULAR_CHARACTER_TYPE>;
using rst=regular::record::Some<REGULAR_CHARACTER_TYPE>;
using rbt=regular::record::Binary<REGULAR_CHARACTER_TYPE>;
using ret=regular::record::Every<REGULAR_CHARACTER_TYPE>;
using rgt=regular::record::Greedy<REGULAR_CHARACTER_TYPE>;

using pt=regular::Pattern<REGULAR_CHARACTER_TYPE>;
using pot=regular::pattern::EmptyString<REGULAR_CHARACTER_TYPE>;
using pct=regular::pattern::LiteralCharacter<REGULAR_CHARACTER_TYPE>;
template<typename Context>
using pcct=regular::pattern::literal_character::Closure<REGULAR_CHARACTER_TYPE, Context>;

auto po = std::make_shared<pot>();

std::shared_ptr<pct> pc(const pattern::LiteralCharacter<REGULAR_CHARACTER_TYPE>::Describe &describe) {
    return std::make_shared<pct>(describe);
}

#endif