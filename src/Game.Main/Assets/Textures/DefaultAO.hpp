#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class DefaultAO :
        public TextureAsset,
        public AssetAutoGuid<DefaultAO> {
    public:
        DefaultAO();
    };

    AUTO_REGISTER_ASSET(DefaultAO)

}
