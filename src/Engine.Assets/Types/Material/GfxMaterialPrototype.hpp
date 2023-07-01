#pragma once
#include "../Asset.hpp"
#include <Engine.Serialization/Access/__fwd.hpp>

namespace hg::engine::assets {
    class GfxMaterialPrototype :
        public Asset {
    public:
        using this_type = GfxMaterialPrototype;

        ACCESS_LAYOUT
        ACCESS_STRUCTURE

    public:
        inline static constexpr asset_type_id typeId { "GfxMaterialPrototype"_typeId };

    public:
        GfxMaterialPrototype(cref<asset_guid> guid_);

    private:
        // TODO: Serialize/Map instantiated parameters
    };
}
