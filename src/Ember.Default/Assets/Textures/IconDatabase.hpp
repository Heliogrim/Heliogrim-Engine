#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconDatabase :
        public TextureAsset,
        public AssetAutoGuid<IconDatabase> {
    public:
        IconDatabase();
    };
}
