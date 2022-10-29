#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconCSharp :
        public TextureAsset,
        public AssetAutoGuid<IconCSharp> {
    public:
        IconCSharp();
    };

    AUTO_REGISTER_ASSET(IconCSharp)

}
