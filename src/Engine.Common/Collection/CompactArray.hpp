#pragma once

#include "AutoArray.hpp"

namespace hg {
    template <typename Type_, typename Allocator_ = std::allocator<Type_>>
    using CompactArray = AutoArray<Type_, Allocator_>;
}
