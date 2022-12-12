#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconCpp :
        public TextureAsset,
        public AssetAutoGuid<IconCpp> {
    public:
        IconCpp();
    };
}
