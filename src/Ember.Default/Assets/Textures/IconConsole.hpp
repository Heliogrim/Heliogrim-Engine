#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconConsole :
        public TextureAsset,
        public AssetAutoGuid<IconConsole> {
    public:
        IconConsole();
    };

    AUTO_REGISTER_ASSET(IconConsole)

}
