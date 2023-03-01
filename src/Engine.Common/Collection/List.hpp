#pragma once

#include "../__macro.hpp"

#include <list>

namespace hg {
    template <class Type_, class Allocator_ = _STD allocator<Type_>>
    using list = _STD list<Type_, Allocator_>;
}
