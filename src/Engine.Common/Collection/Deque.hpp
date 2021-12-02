#pragma once

#include "../__macro.hpp"

#include <deque>

namespace ember {

    template <class Type_, class Allocator_ = _STD allocator<Type_>>
    using deque = _STD deque<Type_, Allocator_>;

}
