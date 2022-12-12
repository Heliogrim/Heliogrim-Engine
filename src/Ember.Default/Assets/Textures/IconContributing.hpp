#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconContributing :
        public TextureAsset,
        public AssetAutoGuid<IconContributing> {
    public:
        IconContributing();
    };
}
