#include "regular.h"

#ifdef REGULAR_CHARACTER_TYPE

using rt=regular::Record<REGULAR_CHARACTER_TYPE>;
using rut=regular::record::Unary<REGULAR_CHARACTER_TYPE>;
using rst=regular::record::Some<REGULAR_CHARACTER_TYPE>;
using rbt=regular::record::Binary<REGULAR_CHARACTER_TYPE>;
using ret=regular::record::Every<REGULAR_CHARACTER_TYPE>;
using rgt=regular::record::Greedy<REGULAR_CHARACTER_TYPE>;

#endif