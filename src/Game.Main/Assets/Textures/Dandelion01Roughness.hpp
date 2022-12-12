#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class Dandelion01Roughness :
        public TextureAsset,
        public AssetAutoGuid<Dandelion01Roughness> {
    public:
        Dandelion01Roughness();
    };
}
