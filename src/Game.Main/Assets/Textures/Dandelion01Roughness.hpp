#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class Dandelion01Roughness :
        public TextureAsset,
        public AssetAutoGuid<Dandelion01Roughness> {
    public:
        Dandelion01Roughness();
    };

    AUTO_REGISTER_ASSET(Dandelion01Roughness)

}
