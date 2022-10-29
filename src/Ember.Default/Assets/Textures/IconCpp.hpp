#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconCpp :
        public TextureAsset,
        public AssetAutoGuid<IconCpp> {
    public:
        IconCpp();
    };

    AUTO_REGISTER_ASSET(IconCpp)

}
