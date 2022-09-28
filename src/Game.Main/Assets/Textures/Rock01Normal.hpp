#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class Rock01Normal :
        public TextureAsset,
        public AssetAutoGuid<Rock01Normal> {
    public:
        Rock01Normal();
    };

    AUTO_REGISTER_ASSET(Rock01Normal)

}
