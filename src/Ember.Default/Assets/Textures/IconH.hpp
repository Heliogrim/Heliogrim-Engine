#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconH :
        public TextureAsset,
        public AssetAutoGuid<IconH> {
    public:
        IconH();
    };

    AUTO_REGISTER_ASSET(IconH)

}
