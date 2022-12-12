#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconTemplate :
        public TextureAsset,
        public AssetAutoGuid<IconTemplate> {
    public:
        IconTemplate();
    };
}
