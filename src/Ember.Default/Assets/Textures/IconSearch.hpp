#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconSearch :
        public TextureAsset,
        public AssetAutoGuid<IconSearch> {
    public:
        IconSearch();
    };

    AUTO_REGISTER_ASSET(IconSearch)

}
