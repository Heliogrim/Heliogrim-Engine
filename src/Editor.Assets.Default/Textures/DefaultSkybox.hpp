#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class DefaultSkybox :
        public TextureAsset,
        public AssetAutoGuid<DefaultSkybox> {
    public:
        DefaultSkybox();
    };
}
