#pragma once

#include <atomic>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Hash/Murmur3.hpp>

namespace ember {

    struct asset_guid {
        uint16_t pred;
        uint8_t b1;
        uint8_t b2;
        uint32_t post;

        /**
         * Cast that converts the given  to a bool
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
    };

    /**
     * Identifier for the invalid asset
     */
    inline constexpr static asset_guid invalid_asset_guid = { 0, 0, 0, 0 };

    /**
     * Generates an asset unique identifier
     *
     * @author Julius
     * @date 23.10.2020
     *
     * @returns The asset unique identifier.
     */
    static asset_guid generate_asset_guid() {
        static _STD atomic_uint_fast32_t ai { 1 };
        const auto post = ai.fetch_add(1, _STD memory_order_relaxed);
        return { 0, 0, 0, post };
    }

}

namespace std {
    template <>
    struct hash<ember::asset_guid> {
        _STD size_t operator()(const ember::asset_guid& value_) const noexcept {
            /**
             * Warning: Don't use identity
             */
            ember::u64 dst[2];
            ember::hash::murmur3_x64_128(
                &value_,
                sizeof(ember::asset_guid),
                0x9FB21C651E98DF25ui32,
                &dst
            );
            return dst[0];
        }
    };
}
