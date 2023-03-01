#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class CerberusAO :
        public TextureAsset,
        public AssetAutoGuid<CerberusAO> {
    public:
        CerberusAO();
    };
}
