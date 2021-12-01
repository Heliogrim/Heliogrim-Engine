#pragma once

#include <vector>

#include "../__macro.hpp"
#include "../Types.hpp"

namespace ember {

    template <typename Type_, typename Alloc_ = _STD allocator<Type_>>
    using Vector = _STD vector<Type_, Alloc_>;
}
