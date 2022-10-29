#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconLib :
        public TextureAsset,
        public AssetAutoGuid<IconLib> {
    public:
        IconLib();
    };

    AUTO_REGISTER_ASSET(IconLib)

}
