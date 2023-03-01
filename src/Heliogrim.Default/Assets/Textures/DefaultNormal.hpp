#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class DefaultNormal :
        public TextureAsset,
        public AssetAutoGuid<DefaultNormal> {
    public:
        DefaultNormal();
    };
}
