#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class Dandelion01Diffuse :
        public TextureAsset,
        public AssetAutoGuid<Dandelion01Diffuse> {
    public:
        Dandelion01Diffuse();
    };
}
