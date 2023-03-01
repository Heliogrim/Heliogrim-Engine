#pragma once

#include <memory>

#include "InlineArray.hpp"

namespace hg {
    template <typename Type_, size_t Capacity_, typename Allocator_ = _STD allocator<Type_>>
    using InlineAutoArray = InlineArray<Type_, Capacity_>;
}
