#pragma once

#include "Asset.hpp"

namespace ember {

    class RenderableAsset :
        public Asset {
    public:
        using base_type = Asset;

        using value_type = RenderableAsset;
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
         * @param  internal_ The internal state.
         */
        RenderableAsset(cref<asset_guid> guid_, cref<asset_type_id> typeId_, ptr<void> internal_) noexcept;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 06.10.2021
         */
        ~RenderableAsset() noexcept;
    };

}
