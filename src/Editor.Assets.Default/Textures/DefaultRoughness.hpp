#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class DefaultRoughness :
        public TextureAsset,
        public AssetAutoGuid<DefaultRoughness> {
    public:
        DefaultRoughness();
    };
}
