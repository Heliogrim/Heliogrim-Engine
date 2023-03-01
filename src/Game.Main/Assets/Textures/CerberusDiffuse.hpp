#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class CerberusDiffuse :
        public TextureAsset,
        public AssetAutoGuid<CerberusDiffuse> {
    public:
        CerberusDiffuse();
    };
}
