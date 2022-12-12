#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconFont :
        public TextureAsset,
        public AssetAutoGuid<IconFont> {
    public:
        IconFont();
    };
}
