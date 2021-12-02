#pragma once

#include <unordered_set>
#include <robin/robin_set.h>

namespace ember {

    template <
        class Type_,
        class Hash_ = _STD hash<Type_>,
        class Equal_ = _STD equal_to<Type_>,
        class Allocator_ = _STD allocator<Type_>>
    using CompactSet = tsl::robin_set<Type_, Hash_, Equal_, Allocator_>;

    template <
        class Type_,
        class Hash_ = _STD hash<Type_>,
        class Equal_ = _STD equal_to<Type_>,
        class Allocator_ = _STD allocator<Type_>>
    using Set = _STD unordered_set<Type_, Hash_, Equal_, Allocator_>;

}
