#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Hash/Murmur3.hpp>

namespace ember {
    struct asset_guid {
        uint16_t pred;
        uint8_t b1;
        uint8_t b2;
        uint32_t post;

        static asset_guid from(const u64 data_) noexcept {
            return asset_guid {
                { static_cast<uint16_t>(data_ >> 48) },
                { static_cast<uint8_t>(data_ >> 40) },
                { static_cast<uint8_t>(data_ >> 32) },
                { static_cast<uint32_t>(data_) }
            };
        }

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
    extern asset_guid generate_asset_guid();
}

namespace std {
    template <>
    struct hash<ember::asset_guid> {
        _STD size_t operator()(const ember::asset_guid& value_) const noexcept {

            constexpr ember::u64 size = sizeof(ember::asset_guid);

            /**
             * Warning: Don't use identity
             */
            ember::u64 dst[2];
            ember::hash::murmur3_x64_128(
                &value_,
                static_cast<ember::u32>(size),
                0x9FB21C651E98DF25ui32,
                &dst
            );
            return dst[0];
        }
    };

    template <>
    struct equal_to<ember::asset_guid> {
        bool operator()(const ember::asset_guid& left_, const ember::asset_guid& right_) const noexcept {
            return left_.as_uint64() == right_.as_uint64();
        }
    };

    template <>
    struct less<ember::asset_guid> {
        bool operator()(const ember::asset_guid& left_, const ember::asset_guid& right_) const noexcept {
            return left_.as_uint64() < right_.as_uint64();
        }
    };
}
