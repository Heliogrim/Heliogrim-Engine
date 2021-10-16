#pragma once

#include "Asset.hpp"
#include "AssetAutoGuid.hpp"
#include "AssetAutoRegister.hpp"

namespace ember {

    class PfxMaterialAsset :
        public Asset {
    public:
        using base_type = Asset;

        using value_type = PfxMaterialAsset;
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
        PfxMaterialAsset(cref<asset_guid> guid_) noexcept;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 06.10.2021
         */
        ~PfxMaterialAsset() noexcept;

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
