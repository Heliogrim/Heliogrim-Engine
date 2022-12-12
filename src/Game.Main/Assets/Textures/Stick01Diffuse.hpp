#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class Stick01Diffuse :
        public TextureAsset,
        public AssetAutoGuid<Stick01Diffuse> {
    public:
        Stick01Diffuse();
    };
}
