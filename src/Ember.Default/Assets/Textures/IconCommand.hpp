#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconCommand :
        public TextureAsset,
        public AssetAutoGuid<IconCommand> {
    public:
        IconCommand();
    };

    AUTO_REGISTER_ASSET(IconCommand)

}
