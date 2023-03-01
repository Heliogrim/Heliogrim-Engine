#pragma once
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class GrassWild01Normal :
        public TextureAsset,
        public AssetAutoGuid<GrassWild01Normal> {
    public:
        GrassWild01Normal();
    };
}
