#pragma once

#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/GfxMaterialAsset.hpp>

namespace hg::game::assets::material {
    class ForestGround01 :
        public GfxMaterialAsset,
        public AssetAutoGuid<ForestGround01> {
    public:
        ForestGround01();
    };
}
