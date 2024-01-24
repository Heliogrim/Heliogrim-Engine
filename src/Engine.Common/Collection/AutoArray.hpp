#pragma once

#include "Vector.hpp"

namespace hg {
    // TODO: Custom Implementation

    template <typename Type_, typename Allocator_ = std::allocator<Type_>>
    using AutoArray = Vector<Type_, Allocator_>;
}
