#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class DryGroundRocks01Roughness :
        public TextureAsset,
        public AssetAutoGuid<DryGroundRocks01Roughness> {
    public:
        DryGroundRocks01Roughness();
    };
}
