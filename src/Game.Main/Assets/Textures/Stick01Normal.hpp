#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class Stick01Normal :
        public TextureAsset,
        public AssetAutoGuid<Stick01Normal> {
    public:
        Stick01Normal();
    };
}
