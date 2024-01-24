#pragma once

#include "../__macro.hpp"

#include <deque>

namespace hg {
    template <class Type_, class Allocator_ = std::allocator<Type_>>
    using Deque = std::deque<Type_, Allocator_>;
}
