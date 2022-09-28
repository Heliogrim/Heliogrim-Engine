#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class DefaultAlpha :
        public TextureAsset,
        public AssetAutoGuid<DefaultAlpha> {
    public:
        DefaultAlpha();
    };

    AUTO_REGISTER_ASSET(DefaultAlpha)

}
