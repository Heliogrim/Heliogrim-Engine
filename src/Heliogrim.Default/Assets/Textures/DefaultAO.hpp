#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class DefaultAO :
        public TextureAsset,
        public AssetAutoGuid<DefaultAO> {
    public:
        DefaultAO();
    };
}
