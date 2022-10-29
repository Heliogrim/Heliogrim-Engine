#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconHpp :
        public TextureAsset,
        public AssetAutoGuid<IconHpp> {
    public:
        IconHpp();
    };

    AUTO_REGISTER_ASSET(IconHpp)

}
