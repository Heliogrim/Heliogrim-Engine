#pragma once

#include "RenderableAsset.hpp"
#include "StreamableAsset.hpp"

namespace ember {
    class StreamableRenderableAsset :
        public RenderableAsset {
    public:
        using base_type = RenderableAsset;

        using value_type = StreamableRenderableAsset;
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
         * @param  typeId_ Identifier for the type.
         * @param  internal_ The internal.
         */
        StreamableRenderableAsset(cref<asset_guid> guid_, cref<asset_type_id> typeId_, ptr<void> internal_) noexcept;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 06.10.2021
         */
        ~StreamableRenderableAsset() noexcept;
    };
}
