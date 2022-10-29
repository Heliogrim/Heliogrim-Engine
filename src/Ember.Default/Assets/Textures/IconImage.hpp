#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconImage :
        public TextureAsset,
        public AssetAutoGuid<IconImage> {
    public:
        IconImage();
    };

    AUTO_REGISTER_ASSET(IconImage)

}
