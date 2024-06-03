#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class DefaultAlpha :
        public TextureAsset,
        public AssetAutoGuid<DefaultAlpha> {
    public:
        DefaultAlpha();
    };
}
