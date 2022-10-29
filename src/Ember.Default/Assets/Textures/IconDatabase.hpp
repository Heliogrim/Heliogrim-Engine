#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconDatabase :
        public TextureAsset,
        public AssetAutoGuid<IconDatabase> {
    public:
        IconDatabase();
    };

    AUTO_REGISTER_ASSET(IconDatabase)

}
