#pragma once
#include <atomic>
#include <Engine.Common/Hash/Murmur3.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

namespace ember {
    /**
     * An guid actor identifier.
     *
     * @author Julius
     * @date 31.08.2020
     */
    typedef struct actor_guid {
        uint16_t pred;
        uint8_t b1;
        uint8_t b2;
        uint32_t post;

        // constexpr actor_guid(cref<actor_guid> other_) noexcept = default;

        /**
         * Cast that converts the given to a bool
         *
         * @author Julius
         * @date 25.10.2020
         *
         * @returns The result of the operation.
         */
        operator bool() const {
            return *(reinterpret_cast<const uint64_t*>(this)) != 0;
        }

        /**
         * Logical negation operator
         *
         * @author Julius
         * @date 25.10.2020
         *
         * @returns The logical inverse of this value.
         */
        bool operator!() const {
            return *(reinterpret_cast<const uint64_t*>(this)) == 0;
        }

        /**
         * Converts this to an uint_64
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns A reference to an uint64_t.
         */
        uint64_t& as_uint64() {
            return *(reinterpret_cast<uint64_t*>(this));
        }

        /**
         * Converts this to an uint_64
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns A reference to a const uint64_t.
         */
        const uint64_t& as_uint64() const {
            return *(reinterpret_cast<const uint64_t*>(this));
        }
    } actor_guid;

    /**
     * Identifier for the invalid actor
     */
    inline constexpr static actor_guid invalid_actor_guid = { 0, 0, 0, 0 };

    /**
     * Equality operator
     *
     * @author Julius
     * @date 23.10.2020
     *
     * @param  left_ The first instance to compare.
     * @param  right_ The second instance to compare.
     *
     * @returns True if the parameters are considered equivalent.
     */
    inline bool operator==(const actor_guid& left_, const actor_guid& right_) {
        return *(reinterpret_cast<const uint64_t*>(&left_)) == *(reinterpret_cast<const uint64_t*>(&right_));
    }

    /**
     * Generates an actor unique identifier
     *
     * @author Julius
     * @date 23.10.2020
     *
     * @returns The actor unique identifier.
     */
    [[maybe_unused]] static actor_guid generate_actor_guid() {
        static _STD atomic_uint_fast32_t ai { 1 };
        const auto post = ai.fetch_add(1, _STD memory_order_relaxed);
        return { 0, 0, 0, post };
    }
}

namespace std {
    /**
     * Specific equality check for actor_guid
     *
     * @author Julius
     * @date 30.10.2020
     */
    template <>
    struct equal_to<ember::actor_guid> {
        bool operator()(const ember::actor_guid& left_,
            const ember::actor_guid& right_) const noexcept {
            return left_.as_uint64() == right_.as_uint64();
        }
    };

    /**
     * Specific less check for actor_guid
     *
     * @author Julius
     * @date 30.10.2020
     */
    template <>
    struct less<ember::actor_guid> {
        bool operator()(const ember::actor_guid& left_,
            const ember::actor_guid& right_) const noexcept {
            return left_.as_uint64() < right_.as_uint64();
        }
    };

    /**
     * Specific hash of actor_guid
     *
     * @author Julius
     * @date 29.10.2020
     */
    template <>
    struct hash<ember::actor_guid> {
        _STD size_t operator()(const ember::actor_guid& value_) const noexcept {
            /**
             * !important! don't use identity
             */
            ember::u64 dst[2];
            ember::hash::murmur3_x64_128(
                &value_,
                sizeof(ember::actor_guid),
                0x9FB21C651E98DF25ui32,
                &dst
            );
            return dst[0];
        }
    };
}
