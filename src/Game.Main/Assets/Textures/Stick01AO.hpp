#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class Stick01AO :
        public TextureAsset,
        public AssetAutoGuid<Stick01AO> {
    public:
        Stick01AO();
    };
}
