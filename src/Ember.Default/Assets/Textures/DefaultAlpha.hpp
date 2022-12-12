#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class DefaultAlpha :
        public TextureAsset,
        public AssetAutoGuid<DefaultAlpha> {
    public:
        DefaultAlpha();
    };
}
