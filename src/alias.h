#pragma once

namespace regular::alias {

#define REGULAR_CHARACTER_TYPE char

    namespace narrow {

#include "ALIAS.h"

}

#undef REGULAR_CHARACTER_TYPE

#define REGULAR_CHARACTER_TYPE wchar_t

namespace wide {

#include "ALIAS.h"

    }

#undef REGULAR_CHARACTER_TYPE
}