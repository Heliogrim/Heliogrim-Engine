#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconFavicon :
        public TextureAsset,
        public AssetAutoGuid<IconFavicon> {
    public:
        IconFavicon();
    };
}
