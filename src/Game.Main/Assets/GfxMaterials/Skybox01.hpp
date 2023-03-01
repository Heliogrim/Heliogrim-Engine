#pragma once

#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/GfxMaterialAsset.hpp>

namespace hg::game::assets::material {
    class Skybox01 :
        public GfxMaterialAsset,
        public AssetAutoGuid<Skybox01> {
    public:
        Skybox01();
    };
}
