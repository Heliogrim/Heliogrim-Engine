#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class Skybox01Diffuse :
        public TextureAsset,
        public AssetAutoGuid<Skybox01Diffuse> {
    public:
        Skybox01Diffuse();
    };
}
