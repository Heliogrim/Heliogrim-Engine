#pragma once

#include <unordered_map>

namespace hg {
    template <
        class Key_,
        class Type_,
        class Hash_ = std::hash<Key_>,
        class Equal_ = std::equal_to<Key_>,
        class Alloc_ = std::allocator<std::pair<const Key_, Type_>>>
    using StableUnorderedMap = std::unordered_map<Key_, Type_, Hash_, Equal_, Alloc_>;
}
