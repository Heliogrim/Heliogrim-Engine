#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class DryGroundRocks01AO :
        public TextureAsset,
        public AssetAutoGuid<DryGroundRocks01AO> {
    public:
        DryGroundRocks01AO();
    };
}
