#pragma once

#include <algorithm>

#include "../__macro.hpp"
#include "../Wrapper.hpp"

namespace ember {
    template <typename KeyType_, class Hasher = _STD hash<KeyType_>>
    struct AssocKey {
        using this_type = AssocKey<KeyType_>;

        using hash_type = size_t;
        using value_type = KeyType_;

        hash_type hash { ~0ui64 };
        value_type value;

        [[nodiscard]] explicit operator value_type() const noexcept {
            return value;
        }

        [[nodiscard]] operator cref<value_type>() const noexcept {
            return value;
        }

        [[nodiscard]] explicit operator value_type() noexcept {
            return value;
        }

        [[nodiscard]] operator ref<value_type>() noexcept = delete;

    public:
        [[nodiscard]] bool operator==(cref<this_type> other_) const noexcept {
            return value == other_.value;
        }

        [[nodiscard]] bool operator!=(cref<this_type> other_) const noexcept {
            return value != other_.value;
        }

    public:
        static this_type from(mref<value_type> value_) {
            return this_type {
                Hasher {}.operator()(value_),
                _STD move(value_)
            };
        }

        static this_type from(cref<value_type> value_) {
            return this_type {
                Hasher {}.operator()(value_),
                value_
            };
        }
    };
}

namespace std {
    template <typename KeyType_>
    struct hash<::ember::AssocKey<KeyType_>> {
        [[nodiscard]] size_t operator()(::ember::cref<::ember::AssocKey<KeyType_>> obj_) const noexcept {
            return obj_.hash;
        }
    };
}
