#pragma once

#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/GfxMaterialAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace hg::game::assets::material {
    class Cerberus :
        public GfxMaterialAsset,
        public AssetAutoGuid<Cerberus> {
    public:
        Cerberus();
    };
}
