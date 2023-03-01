#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class DryGroundRocks01AO :
        public TextureAsset,
        public AssetAutoGuid<DryGroundRocks01AO> {
    public:
        DryGroundRocks01AO();
    };
}
