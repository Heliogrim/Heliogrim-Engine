#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class Rock01Roughness :
        public TextureAsset,
        public AssetAutoGuid<Rock01Roughness> {
    public:
        Rock01Roughness();
    };
}
