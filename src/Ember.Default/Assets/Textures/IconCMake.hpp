#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconCMake :
        public TextureAsset,
        public AssetAutoGuid<IconCMake> {
    public:
        IconCMake();
    };
}
