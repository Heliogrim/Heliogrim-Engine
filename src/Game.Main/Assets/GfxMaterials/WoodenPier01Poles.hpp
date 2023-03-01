#pragma once

#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/GfxMaterialAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace hg::game::assets::material {
    class WoodenPier01Poles :
        public GfxMaterialAsset,
        public AssetAutoGuid<WoodenPier01Poles> {
    public:
        WoodenPier01Poles();
    };
}
