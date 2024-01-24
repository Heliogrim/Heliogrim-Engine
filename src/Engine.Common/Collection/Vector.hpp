#pragma once

#include <vector>

#include "../__macro.hpp"
#include "../Types.hpp"

namespace hg {
    template <typename Type_, typename Allocator_ = std::allocator<Type_>>
    using Vector = std::vector<Type_, Allocator_>;
}
