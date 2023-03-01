#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Hash/Murmur3.hpp>
#include <Engine.Common/Guid.hpp>

namespace hg {
    struct asset_guid : public Guid {
        using Guid::Guid;
    };

    /**
     * Identifier for the invalid asset
     */
    inline constexpr static asset_guid invalid_asset_guid {};

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
    struct hash<hg::asset_guid> {
        _STD size_t operator()(const hg::asset_guid& value_) const noexcept {

            constexpr hg::u64 size = sizeof(hg::asset_guid);

            /**
             * Warning: Don't use identity
             */
            hg::u64 dst[2];
            hg::hash::murmur3_x64_128(
                &value_,
                static_cast<hg::u32>(size),
                0x9FB21C651E98DF25ui32,
                &dst
            );
            return dst[0];
        }
    };
}
