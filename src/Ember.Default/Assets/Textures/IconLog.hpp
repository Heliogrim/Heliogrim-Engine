#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconLog :
        public TextureAsset,
        public AssetAutoGuid<IconLog> {
    public:
        IconLog();
    };
}
