#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class CerberusRoughness :
        public TextureAsset,
        public AssetAutoGuid<CerberusRoughness> {
    public:
        CerberusRoughness();
    };
}
