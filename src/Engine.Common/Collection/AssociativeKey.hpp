#pragma once

#include <algorithm>

#include "../__macro.hpp"

namespace ember {

    template <typename KeyType_, class Hasher = _STD hash<KeyType_>>
    struct AssocKey {
        using this_type = AssocKey<KeyType_>;

        using hash_type = size_t;
        using value_type = KeyType_;

        hash_type hash { ~0ui64 };
        value_type value;
    };

}
