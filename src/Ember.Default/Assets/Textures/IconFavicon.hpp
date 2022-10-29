#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconFavicon :
        public TextureAsset,
        public AssetAutoGuid<IconFavicon> {
    public:
        IconFavicon();
    };

    AUTO_REGISTER_ASSET(IconFavicon)

}
