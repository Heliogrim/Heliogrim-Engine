#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconLua :
        public TextureAsset,
        public AssetAutoGuid<IconLua> {
    public:
        IconLua();
    };
}
