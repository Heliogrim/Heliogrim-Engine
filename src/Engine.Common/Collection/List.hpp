#pragma once

#include "../__macro.hpp"

#include <deque>
#include <list>
#include <queue>
#include <stack>
#include <vector>

namespace ember {
    template <typename Ty, typename Alloc = _STD allocator<Ty>>
    using vector = _STD vector<Ty, Alloc>;

    template <typename Ty>
    using list = _STD list<Ty>;

    template <typename Ty>
    using queue = _STD queue<Ty>;

    template <typename Ty>
    using stack = _STD stack<Ty>;
}
