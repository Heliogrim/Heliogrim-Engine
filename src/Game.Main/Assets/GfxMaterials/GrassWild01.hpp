#pragma once

#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/GfxMaterialAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace hg::game::assets::material {
    class GrassWild01 :
        public GfxMaterialAsset,
        public AssetAutoGuid<GrassWild01> {
    public:
        GrassWild01();
    };
}
