#pragma once

#include <Engine.Serialization/Access/__fwd.hpp>
#include "Asset.hpp"

namespace hg::engine::assets {
    class AccelEffect :
        public InheritMeta<AccelEffect, Asset> {
    public:
        using this_type = AccelEffect;

        ACCESS_LAYOUT
        ACCESS_STRUCTURE

    public:
        inline static constexpr asset_type_id typeId { "AccelEffect"_typeId };

    public:
        AccelEffect(mref<asset_guid> guid_);
    };
}
