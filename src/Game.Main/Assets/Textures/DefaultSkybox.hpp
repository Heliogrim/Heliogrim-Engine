#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class DefaultSkybox :
        public TextureAsset,
        public AssetAutoGuid<DefaultSkybox> {
    public:
        DefaultSkybox();
    };
}
