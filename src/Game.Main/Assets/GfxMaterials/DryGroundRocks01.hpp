#pragma once

#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/GfxMaterialAsset.hpp>

namespace hg::game::assets::material {
    class DryGroundRocks01 :
        public GfxMaterialAsset,
        public AssetAutoGuid<DryGroundRocks01> {
    public:
        DryGroundRocks01();
    };
}
