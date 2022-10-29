#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconArchitecture :
        public TextureAsset,
        public AssetAutoGuid<IconArchitecture> {
    public:
        IconArchitecture();
    };

    AUTO_REGISTER_ASSET(IconArchitecture)

}
