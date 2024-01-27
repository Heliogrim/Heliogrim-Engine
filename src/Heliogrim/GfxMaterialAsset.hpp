#pragma once

#include "StreamableRenderableAsset.hpp"

namespace hg {
    class GfxMatProtoAsset;

    class GfxMaterialAsset :
        public StreamableRenderableAsset {
    public:
        using base_type = StreamableRenderableAsset;

        using value_type = GfxMaterialAsset;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        GfxMaterialAsset(
            mref<asset_guid> guid_,
            mref<asset_guid> prototypeGuid_
        ) noexcept;

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

    public:
        [[nodiscard]] const ptr<const GfxMatProtoAsset> prototype() const noexcept;
    };
}
