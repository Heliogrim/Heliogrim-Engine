#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class WoodenPier01PlanksDiffuse :
        public TextureAsset,
        public AssetAutoGuid<WoodenPier01PlanksDiffuse> {
    public:
        WoodenPier01PlanksDiffuse();
    };
}
