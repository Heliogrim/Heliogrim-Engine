#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconTree :
        public TextureAsset,
        public AssetAutoGuid<IconTree> {
    public:
        IconTree();
    };
}
