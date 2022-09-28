#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class DefaultDiffuse :
        public TextureAsset,
        public AssetAutoGuid<DefaultDiffuse> {
    public:
        DefaultDiffuse();
    };

    AUTO_REGISTER_ASSET(DefaultDiffuse)

}
