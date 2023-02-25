#pragma once

#include <unordered_map>

namespace ember {
    template <
        class Key_,
        class Type_,
        class Hash_ = _STD hash<Key_>,
        class Equal_ = _STD equal_to<Key_>,
        class Alloc_ = _STD allocator<_STD pair<const Key_, Type_>>>
    using StableUnorderedMap = _STD unordered_map<Key_, Type_, Hash_, Equal_, Alloc_>;
}
