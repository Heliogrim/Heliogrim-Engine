#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class Cannon01Roughness :
        public TextureAsset,
        public AssetAutoGuid<Cannon01Roughness> {
    public:
        Cannon01Roughness();
    };
}
