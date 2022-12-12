#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconH :
        public TextureAsset,
        public AssetAutoGuid<IconH> {
    public:
        IconH();
    };
}
