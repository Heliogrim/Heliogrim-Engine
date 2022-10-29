#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconC :
        public TextureAsset,
        public AssetAutoGuid<IconC> {
    public:
        IconC();
    };

    AUTO_REGISTER_ASSET(IconC)

}
