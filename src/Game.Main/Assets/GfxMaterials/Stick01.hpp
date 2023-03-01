#pragma once

#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/GfxMaterialAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace hg::game::assets::material {
    class Stick01 :
        public GfxMaterialAsset,
        public AssetAutoGuid<Stick01> {
    public:
        Stick01();
    };
}
