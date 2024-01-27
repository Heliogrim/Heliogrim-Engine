#pragma once

#include "Asset.hpp"
#include "AssetAutoGuid.hpp"

namespace hg {
    class SfxMaterialAsset :
        public Asset {
    public:
        using base_type = Asset;

        using value_type = SfxMaterialAsset;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    protected:
        /**
         * Constructor
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @param  guid_ Unique identifier.
         */
        SfxMaterialAsset(mref<asset_guid> guid_) noexcept;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 06.10.2021
         */
        ~SfxMaterialAsset() noexcept;

    public:
        /**
         * Query if this is valid type
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @returns True if valid type, false if not.
         */
        [[nodiscard]] bool isValidType() const noexcept;
    };
}
