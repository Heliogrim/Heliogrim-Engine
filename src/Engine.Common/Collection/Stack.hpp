#pragma once

#include "../__macro.hpp"

#include <stack>

namespace hg {
    template <class Type_, class ContainerType_ = std::deque<Type_>>
    using Stack = std::stack<Type_, ContainerType_>;
}
