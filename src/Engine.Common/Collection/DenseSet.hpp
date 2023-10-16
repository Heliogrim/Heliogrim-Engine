#pragma once

#include <Engine.Common/Wrapper.hpp>

#include <ankerl/unordered_dense.h>

namespace hg {
    template <
        class Type_,
        class Hash_ = ankerl::unordered_dense::v4_1_2::hash<Type_>,
        class Equal_ = _STD equal_to<Type_>,
        class Allocator_ = _STD allocator<Type_>>
    using DenseSet = ankerl::unordered_dense::set<Type_, Hash_, Equal_, Allocator_>;
}
