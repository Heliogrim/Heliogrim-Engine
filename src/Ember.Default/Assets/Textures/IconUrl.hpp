#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconUrl :
        public TextureAsset,
        public AssetAutoGuid<IconUrl> {
    public:
        IconUrl();
    };

    AUTO_REGISTER_ASSET(IconUrl)

}
