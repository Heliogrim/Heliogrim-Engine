#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class DefaultMetalness :
        public TextureAsset,
        public AssetAutoGuid<DefaultMetalness> {
    public:
        DefaultMetalness();
    };
}
