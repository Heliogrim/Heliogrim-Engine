#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconReadme :
        public TextureAsset,
        public AssetAutoGuid<IconReadme> {
    public:
        IconReadme();
    };
}
