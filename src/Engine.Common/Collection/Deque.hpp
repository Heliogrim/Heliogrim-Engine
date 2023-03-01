#pragma once

#include "../__macro.hpp"

#include <deque>

namespace hg {
    template <class Type_, class Allocator_ = _STD allocator<Type_>>
    using Deque = _STD deque<Type_, Allocator_>;
}
