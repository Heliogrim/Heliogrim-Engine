#pragma once
#include <Heliogrim/GfxMaterialAsset.hpp>

namespace hg::game::assets::material {
    class DefaultMaterial :
        public GfxMaterialAsset,
        public AssetAutoGuid<DefaultMaterial> {
    public:
        inline static constexpr type_id typeId { "material::gfx::DefaultMaterial"_typeId };

    public:
        DefaultMaterial();
    };
}
