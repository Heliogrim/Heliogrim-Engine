#pragma once

#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/GfxMaterialAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace hg::game::assets::material {
    class WoodenBucket01 :
        public GfxMaterialAsset,
        public AssetAutoGuid<WoodenBucket01> {
    public:
        WoodenBucket01();
    };
}
