#pragma once

#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/GfxMaterialAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace hg::game::assets::material {
    class Dandelion01 :
        public GfxMaterialAsset,
        public AssetAutoGuid<Dandelion01> {
    public:
        Dandelion01();
    };
}
