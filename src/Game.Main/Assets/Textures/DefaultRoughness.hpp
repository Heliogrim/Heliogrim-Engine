#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class DefaultRoughness :
        public TextureAsset,
        public AssetAutoGuid<DefaultRoughness> {
    public:
        DefaultRoughness();
    };
}
