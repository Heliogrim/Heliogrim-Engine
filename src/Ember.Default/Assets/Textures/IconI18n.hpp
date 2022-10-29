#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconI18n :
        public TextureAsset,
        public AssetAutoGuid<IconI18n> {
    public:
        IconI18n();
    };

    AUTO_REGISTER_ASSET(IconI18n)

}
