#pragma once

#include <Engine.Serialization/Access/__fwd.hpp>
#include "../Asset.hpp"

namespace hg::engine::assets {
    class GfxMaterial :
        public InheritMeta<GfxMaterial, Asset> {
    public:
        using this_type = GfxMaterial;

        ACCESS_LAYOUT
        ACCESS_STRUCTURE

    public:
        inline static constexpr asset_type_id typeId { "GfxMaterial"_typeId };

    public:
        GfxMaterial(cref<asset_guid> guid_);

    private:
        asset_guid _prototype;

    public:
        [[nodiscard]] asset_guid prototype() const noexcept;

        // TODO: Serialize/Map instantiated parameters
    };
}
