#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconLib :
        public TextureAsset,
        public AssetAutoGuid<IconLib> {
    public:
        IconLib();
    };
}
