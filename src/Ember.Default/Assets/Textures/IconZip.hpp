#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconZip :
        public TextureAsset,
        public AssetAutoGuid<IconZip> {
    public:
        IconZip();
    };

    AUTO_REGISTER_ASSET(IconZip)

}
