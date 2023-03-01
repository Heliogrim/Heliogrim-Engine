#pragma once
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class GrassWild01Displacement :
        public TextureAsset,
        public AssetAutoGuid<GrassWild01Displacement> {
    public:
        GrassWild01Displacement();
    };
}
