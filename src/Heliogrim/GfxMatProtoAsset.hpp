#pragma once

#include "Asset.hpp"
#include "AssetAutoGuid.hpp"

namespace hg {
    class GfxMatProtoAsset :
        public Asset {
    public:
        using base_type = Asset;

        using value_type = GfxMatProtoAsset;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        GfxMatProtoAsset(cref<asset_guid> guid_) noexcept;

        ~GfxMatProtoAsset() noexcept;

    public:
        [[nodiscard]] bool isValidType() const noexcept;
    };
}
