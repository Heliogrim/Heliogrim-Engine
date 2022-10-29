#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconLua :
        public TextureAsset,
        public AssetAutoGuid<IconLua> {
    public:
        IconLua();
    };

    AUTO_REGISTER_ASSET(IconLua)

}
