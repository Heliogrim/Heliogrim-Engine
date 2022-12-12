#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconUrl :
        public TextureAsset,
        public AssetAutoGuid<IconUrl> {
    public:
        IconUrl();
    };
}
