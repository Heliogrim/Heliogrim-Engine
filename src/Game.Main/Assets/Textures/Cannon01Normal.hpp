#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class Cannon01Normal :
        public TextureAsset,
        public AssetAutoGuid<Cannon01Normal> {
    public:
        Cannon01Normal();
    };
}
