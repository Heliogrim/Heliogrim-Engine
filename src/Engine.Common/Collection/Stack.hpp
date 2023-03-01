#pragma once

#include "../__macro.hpp"

#include <stack>

namespace hg {
    template <class Type_, class ContainerType_ = _STD deque<Type_>>
    using Stack = _STD stack<Type_, ContainerType_>;
}
