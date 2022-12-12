#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconDocument :
        public TextureAsset,
        public AssetAutoGuid<IconDocument> {
    public:
        IconDocument();
    };
}
