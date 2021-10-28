#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Assets/AssetTypeId.hpp>

namespace ember {

    template <typename AssetType_>
    struct ComponentAssetHandle {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 28.10.2021
         */
        ComponentAssetHandle() noexcept = default;

    public:
        asset_guid assetGuid;
        asset_type_id assetTypeId;
    };

}
