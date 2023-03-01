#pragma once
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class GrassWild01AO :
        public TextureAsset,
        public AssetAutoGuid<GrassWild01AO> {
    public:
        GrassWild01AO();
    };
}
