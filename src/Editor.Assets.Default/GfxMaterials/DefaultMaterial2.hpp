#pragma once
#include <Heliogrim/GfxMaterialAsset.hpp>

namespace hg::game::assets::material {
    class DefaultMaterial2 :
        public GfxMaterialAsset,
        public AssetAutoGuid<DefaultMaterial2> {
    public:
        inline static constexpr type_id typeId { "material::gfx::DefaultMaterial2"_typeId };

    public:
        DefaultMaterial2();
    };
}
