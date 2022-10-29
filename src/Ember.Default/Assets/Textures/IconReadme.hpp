#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class IconReadme :
        public TextureAsset,
        public AssetAutoGuid<IconReadme> {
    public:
        IconReadme();
    };

    AUTO_REGISTER_ASSET(IconReadme)

}
