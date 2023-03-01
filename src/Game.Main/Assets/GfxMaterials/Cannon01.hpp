#pragma once

#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/GfxMaterialAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace hg::game::assets::material {
    class Cannon01 :
        public GfxMaterialAsset,
        public AssetAutoGuid<Cannon01> {
    public:
        Cannon01();
    };
}
