#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

namespace hg {
    template <typename AssetType_>
    class AssetAutoGuid {
    public:
        /**
         * Gets the unique identifier for this AssetType_
         *
         * @author Julius
         * @date 16.10.2021
         *
         * @returns A stable asset_guid based on the typeId
         */
        template <typename Type_ = AssetType_> requires _STD is_same_v<Type_, AssetType_> && HasStaticType<Type_>
        [[nodiscard]] static constexpr asset_guid auto_guid() noexcept {
            constexpr hg::type_id stableType { Type_::typeId };
            return asset_guid {
                (u16)(stableType.data),
                (u8)(stableType.data >> 16),
                (u8)(stableType.data >> 24),
                (u32)(stableType.data >> 32)
            };
        }

        /**
         * Gets the unstable unique identifier for this AssetType_
         *
         * @author Julius
         * @date 16.10.2021
         *
         * @details The provided asset_guid uses an unstable generator,
         *  which results in different asset_guids when comparing two executions.
         *  Therefor `unstable_auto_guid` should not be used for assets which are referenced
         *  by any serialized / persistent object.
         *
         * @returns A const reference to the unstable asset_guid
         */
        [[nodiscard]] static cref<asset_guid> unstable_auto_guid() noexcept {
            static asset_guid guid = generate_unstable_asset_guid();
            return guid;
        }
    };
}
