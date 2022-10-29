#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconJson :
        public TextureAsset,
        public AssetAutoGuid<IconJson> {
    public:
        IconJson();
    };

    AUTO_REGISTER_ASSET(IconJson)

}
