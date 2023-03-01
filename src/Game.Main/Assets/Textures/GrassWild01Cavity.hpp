#pragma once
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class GrassWild01Cavity :
        public TextureAsset,
        public AssetAutoGuid<GrassWild01Cavity> {
    public:
        GrassWild01Cavity();
    };
}
