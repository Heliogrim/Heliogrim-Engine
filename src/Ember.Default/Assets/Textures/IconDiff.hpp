#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconDiff :
        public TextureAsset,
        public AssetAutoGuid<IconDiff> {
    public:
        IconDiff();
    };
}
