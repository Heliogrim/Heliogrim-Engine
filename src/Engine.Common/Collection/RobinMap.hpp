#pragma once

#include <robin/robin_map.h>

namespace ember {

    template <
        class Key_,
        class Type_,
        class Hash_ = _STD hash<Key_>,
        class Equal_ = _STD equal_to<Key_>,
        class Alloc_ = _STD allocator<_STD pair<const Key_, Type_>>>
    using RobinMap = tsl::robin_map<Key_, Type_, Hash_, Equal_, Alloc_>;
}
