#pragma once

#include <Engine.Common/__macro.hpp>

START_SUPPRESS_WARNINGS
#include <robin/robin_map.h>
STOP_SUPPRESS_WARNINGS

namespace hg {
    template <
        class Key_,
        class Type_,
        class Hash_ = std::hash<Key_>,
        class Equal_ = std::equal_to<Key_>,
        class Alloc_ = std::allocator<std::pair<const Key_, Type_>>>
    using RobinMap = tsl::robin_map<Key_, Type_, Hash_, Equal_, Alloc_>;
}
