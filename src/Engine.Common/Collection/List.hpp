#pragma once

#include "../__macro.hpp"

#include <list>

namespace hg {
    template <class Type_, class Allocator_ = std::allocator<Type_>>
    using list = std::list<Type_, Allocator_>;
}
