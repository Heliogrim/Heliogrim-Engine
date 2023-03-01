#pragma once

#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/GfxMaterialAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace hg::game::assets::material {
    class Rock01 :
        public GfxMaterialAsset,
        public AssetAutoGuid<Rock01> {
    public:
        Rock01();
    };
}
