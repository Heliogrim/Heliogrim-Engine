#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconCMake :
        public TextureAsset,
        public AssetAutoGuid<IconCMake> {
    public:
        IconCMake();
    };

    AUTO_REGISTER_ASSET(IconCMake)

}
