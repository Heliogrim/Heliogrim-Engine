#pragma once

#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/GfxMaterialAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace hg::game::assets::material {
    class WoodenBucket02 :
        public GfxMaterialAsset,
        public AssetAutoGuid<WoodenBucket02> {
    public:
        WoodenBucket02();
    };
}
