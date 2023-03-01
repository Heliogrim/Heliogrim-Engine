#pragma once

#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/GfxMaterialAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace hg::game::assets::material {
    class WoodenPier01Planks :
        public GfxMaterialAsset,
        public AssetAutoGuid<WoodenPier01Planks> {
    public:
        WoodenPier01Planks();
    };
}
