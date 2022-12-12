#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconCSharp :
        public TextureAsset,
        public AssetAutoGuid<IconCSharp> {
    public:
        IconCSharp();
    };
}
