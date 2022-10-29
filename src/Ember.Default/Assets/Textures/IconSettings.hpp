#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconSettings :
        public TextureAsset,
        public AssetAutoGuid<IconSettings> {
    public:
        IconSettings();
    };

    AUTO_REGISTER_ASSET(IconSettings)

}
