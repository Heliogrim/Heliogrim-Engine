#pragma once

#include <Engine.Common/String.hpp>

#include "Asset.hpp"
#include "AssetAutoGuid.hpp"

namespace hg {
    class FontAsset :
        public Asset {
    public:
        using base_type = Asset;
        using this_type = FontAsset;

    protected:
        FontAsset(cref<asset_guid> guid_) noexcept;

        FontAsset(cref<asset_guid> guid_, cref<string> url_) noexcept;

    public:
        ~FontAsset() noexcept;

    public:
        [[nodiscard]] cref<string> url() const noexcept;
    };
}
