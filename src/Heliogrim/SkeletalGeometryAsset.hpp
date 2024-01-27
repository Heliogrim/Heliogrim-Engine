#pragma once

#include "StreamableRenderableAsset.hpp"

namespace hg {
    class SkeletalGeometryAsset :
        public StreamableRenderableAsset {
    public:
        using base_type = StreamableRenderableAsset;

        using value_type = SkeletalGeometryAsset;
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
        SkeletalGeometryAsset(mref<asset_guid> guid_) noexcept;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 06.10.2021
         */
        ~SkeletalGeometryAsset() noexcept;

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
