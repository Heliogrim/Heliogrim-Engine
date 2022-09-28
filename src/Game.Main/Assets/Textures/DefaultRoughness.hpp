#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class DefaultRoughness :
        public TextureAsset,
        public AssetAutoGuid<DefaultRoughness> {
    public:
        DefaultRoughness();
    };

    AUTO_REGISTER_ASSET(DefaultRoughness)

}
