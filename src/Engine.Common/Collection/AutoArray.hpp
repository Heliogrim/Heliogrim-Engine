#pragma once

#include "Vector.hpp"

namespace ember {
    // TODO: Custom Implementation

    template <typename Type_, typename Allocator_ = _STD allocator<Type_>>
    using AutoArray = Vector<Type_, Allocator_>;
}
