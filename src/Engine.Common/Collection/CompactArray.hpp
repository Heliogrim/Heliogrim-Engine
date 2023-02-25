#pragma once

#include "AutoArray.hpp"

namespace ember {
    template <typename Type_, typename Allocator_ = _STD allocator<Type_>>
    using CompactArray = AutoArray<Type_, Allocator_>;
}
