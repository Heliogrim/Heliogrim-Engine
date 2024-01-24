#pragma once

#include <limits>
#include <unordered_set>
#include <ankerl/unordered_dense.h>

namespace hg {
    template <
        class Type_,
        class Hash_ = ankerl::unordered_dense::v4_1_2::hash<Type_>,
        class Equal_ = std::equal_to<Type_>,
        class Allocator_ = std::allocator<Type_>>
    using CompactSet = ankerl::unordered_dense::set<Type_, Hash_, Equal_, Allocator_>;

    template <
        class Type_,
        class Hash_ = std::hash<Type_>,
        class Equal_ = std::equal_to<Type_>,
        class Allocator_ = std::allocator<Type_>>
    using Set = std::unordered_set<Type_, Hash_, Equal_, Allocator_>;
}
