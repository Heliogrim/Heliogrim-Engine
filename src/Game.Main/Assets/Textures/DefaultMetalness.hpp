#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class DefaultMetalness :
        public TextureAsset,
        public AssetAutoGuid<DefaultMetalness> {
    public:
        DefaultMetalness();
    };

    AUTO_REGISTER_ASSET(DefaultMetalness)

}
