#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class CerberusMetalness :
        public TextureAsset,
        public AssetAutoGuid<CerberusMetalness> {
    public:
        CerberusMetalness();
    };
}
