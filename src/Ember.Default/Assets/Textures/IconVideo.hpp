#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconVideo :
        public TextureAsset,
        public AssetAutoGuid<IconVideo> {
    public:
        IconVideo();
    };
}
