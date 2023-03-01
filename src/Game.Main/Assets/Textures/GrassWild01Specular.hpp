#pragma once
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class GrassWild01Specular :
        public TextureAsset,
        public AssetAutoGuid<GrassWild01Specular> {
    public:
        GrassWild01Specular();
    };
}
