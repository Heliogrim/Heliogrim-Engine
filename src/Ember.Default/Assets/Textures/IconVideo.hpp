#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconVideo :
        public TextureAsset,
        public AssetAutoGuid<IconVideo> {
    public:
        IconVideo();
    };

    AUTO_REGISTER_ASSET(IconVideo)

}
