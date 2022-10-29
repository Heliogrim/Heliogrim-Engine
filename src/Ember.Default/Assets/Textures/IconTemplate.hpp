#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconTemplate :
        public TextureAsset,
        public AssetAutoGuid<IconTemplate> {
    public:
        IconTemplate();
    };

    AUTO_REGISTER_ASSET(IconTemplate)

}
