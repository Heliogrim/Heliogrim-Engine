#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class DefaultNormal :
        public TextureAsset,
        public AssetAutoGuid<DefaultNormal> {
    public:
        DefaultNormal();
    };

    AUTO_REGISTER_ASSET(DefaultNormal)

}
