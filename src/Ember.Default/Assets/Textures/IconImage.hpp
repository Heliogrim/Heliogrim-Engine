#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconImage :
        public TextureAsset,
        public AssetAutoGuid<IconImage> {
    public:
        IconImage();
    };
}
