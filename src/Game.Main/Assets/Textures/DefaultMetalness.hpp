#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class DefaultMetalness :
        public TextureAsset,
        public AssetAutoGuid<DefaultMetalness> {
    public:
        DefaultMetalness();
    };
}
