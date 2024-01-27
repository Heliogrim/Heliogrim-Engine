#pragma once

#include <Engine.Common/String.hpp>

#include "Asset.hpp"
#include "AssetAutoGuid.hpp"

namespace hg {
    class ImageAsset :
        public Asset {
    public:
        using base_type = Asset;

        using value_type = ImageAsset;
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
        ImageAsset(mref<asset_guid> guid_) noexcept;

        /**
         * Constructor
         *
         * @author Julius
         * @date 27.06.2022
         *
         * @param  guid_ Unique identifier.
         * @param  url_ The url to the underlying data resource
         */
        ImageAsset(mref<asset_guid> guid_, cref<string> url_) noexcept;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 06.10.2021
         */
        ~ImageAsset() noexcept;

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

    public:
        [[nodiscard]] cref<string> url() const noexcept;
    };
}
