#pragma once

#include "../__macro.hpp"

#include <stack>

namespace ember {

    template <class Type_, class ContainerType_ = _STD deque<Type_>>
    using stack = _STD stack<Type_, ContainerType_>;

}
