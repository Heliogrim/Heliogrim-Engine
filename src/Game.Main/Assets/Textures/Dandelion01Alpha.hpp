#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class Dandelion01Alpha :
        public TextureAsset,
        public AssetAutoGuid<Dandelion01Alpha> {
    public:
        Dandelion01Alpha();
    };

    AUTO_REGISTER_ASSET(Dandelion01Alpha)

}
