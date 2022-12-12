#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class DryGroundRocks01Diffuse :
        public TextureAsset,
        public AssetAutoGuid<DryGroundRocks01Diffuse> {
    public:
        DryGroundRocks01Diffuse();
    };
}
