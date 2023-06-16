#pragma once

#include <algorithm>

#include "../__macro.hpp"
#include "../Wrapper.hpp"

namespace hg {
    template <typename KeyType_, class Hasher_ = _STD hash<KeyType_>>
    struct AssocKey {
        using this_type = AssocKey<KeyType_, Hasher_>;

        using hash_type = size_t;
        using value_type = KeyType_;

    public:
        template <typename KeyTx_ = KeyType_> requires _STD is_default_constructible_v<KeyTx_>
        constexpr AssocKey() noexcept(_STD is_nothrow_default_constructible_v<KeyTx_>) :
            hash(~0ui64),
            value() {}

        constexpr AssocKey(mref<hash_type> hash_, mref<value_type> key_) :
            hash(_STD move(hash_)),
            value(_STD move(key_)) {}

        constexpr AssocKey(cref<hash_type> hash_, cref<value_type> key_) :
            hash(hash_),
            value(key_) {}

        template <class HasherTx_ = _STD hash<KeyType_>> requires (not _STD is_same_v<Hasher_, HasherTx_>)
        constexpr AssocKey(mref<AssocKey<KeyType_, HasherTx_>> other_) :
            hash(Hasher_ {}.operator()(other_.value)),
            value(_STD move(other_.value)) {}

        template <class HasherTx_ = _STD hash<KeyType_>> requires _STD is_same_v<Hasher_, HasherTx_>
        constexpr AssocKey(mref<AssocKey<KeyType_, HasherTx_>> other_) :
            hash(_STD move(other_.hash)),
            value(_STD move(other_.value)) {}

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
        constexpr static this_type from(mref<value_type> value_) {
            return this_type {
                Hasher_ {}.operator()(value_),
                _STD move(value_)
            };
        }

        constexpr static this_type from(cref<value_type> value_) {
            return this_type {
                Hasher_ {}.operator()(value_),
                value_
            };
        }
    };

    /**/
}

namespace std {
    template <typename KeyType_, typename Hasher_>
    struct hash<::hg::AssocKey<KeyType_, Hasher_>> {
        [[nodiscard]] constexpr size_t operator()(::hg::cref<::hg::AssocKey<KeyType_, Hasher_>> obj_) const noexcept {
            return obj_.hash;
        }
    };
}
