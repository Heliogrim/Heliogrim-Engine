#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconContributing :
        public TextureAsset,
        public AssetAutoGuid<IconContributing> {
    public:
        IconContributing();
    };

    AUTO_REGISTER_ASSET(IconContributing)

}
