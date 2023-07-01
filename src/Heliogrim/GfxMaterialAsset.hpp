#pragma once

#include "StreamableRenderableAsset.hpp"

namespace hg {
    class TextureAsset;

    class GfxMaterialAsset :
        public StreamableRenderableAsset {
    public:
        using base_type = StreamableRenderableAsset;

        using value_type = GfxMaterialAsset;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @param  guid_ Unique identifier.
         */
        GfxMaterialAsset(cref<asset_guid> guid_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 06.10.2021
         */
        ~GfxMaterialAsset() noexcept;

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
