#pragma once
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class GrassWild01Gloss :
        public TextureAsset,
        public AssetAutoGuid<GrassWild01Gloss> {
    public:
        GrassWild01Gloss();
    };
}
