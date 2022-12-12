#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class IconHpp :
        public TextureAsset,
        public AssetAutoGuid<IconHpp> {
    public:
        IconHpp();
    };
}
