#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconLog :
        public TextureAsset,
        public AssetAutoGuid<IconLog> {
    public:
        IconLog();
    };

    AUTO_REGISTER_ASSET(IconLog)

}
