#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class CerberusNormal :
        public TextureAsset,
        public AssetAutoGuid<CerberusNormal> {
    public:
        CerberusNormal();
    };
}
