#pragma once

#include <Engine.Common/Wrapper.hpp>

#include <ankerl/unordered_dense.h>

namespace hg {
    template <
        class Key_,
        class Type_,
        class Hash_ = ankerl::unordered_dense::v4_1_2::hash<Key_>,
        class Equal_ = _STD equal_to<Key_>,
        class Alloc_ = _STD allocator<_STD pair<Key_, Type_>>>
    using DenseMap = ankerl::unordered_dense::map<Key_, Type_, Hash_, Equal_, Alloc_>;
}
